#include "projectile_aim.h"
#include <cmath>
#include <math.h>
#include <algorithm>

void c_projectile_aim::run(c_user_cmd* cmd) {
	if (!config->m_aimbot_global.m_enabled || !config->m_projectile_aim.m_enabled || !ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon)
		return;

	if (!projectile_sim->get_info(ctx->m_local_player, ctx->m_local_weapon, vector(), m_projectile_info))
		return;

	if (!find_ideal_target(ctx->m_local_player))
		return;

	if (!config->m_aimbot_global.m_aim_key.is_held())
		return;

	if (!ctx->m_local_weapon->can_shoot())
		return;

	if (config->m_aimbot_global.m_aim_key.is_held() && (ctx->m_local_player->m_player_class() == CLASS_SNIPER || (ctx->m_local_weapon->get_slot() == 1 && ctx->m_local_player->m_player_class() == CLASS_DEMOMAN)))
		cmd->m_buttons |= IN_ATTACK;

	if (m_target->is_player())
		handle_player_aim(ctx->m_local_player, ctx->m_local_weapon, cmd);

	else if (m_target->is_building())
		handle_building_aim(ctx->m_local_player, ctx->m_local_weapon, cmd);

	// probaly no work becous of this fixthis
	/*	if (m_target && m_target->is_player() && HandleSplashBot(static_cast<c_base_player*>(m_target), ctx->m_local_player->get_shoot_position(), m_target->get_world_space_center(), m_time))
		return;*/

	if (config->m_aimbot.m_plain) {
		ctx->m_engine_client->set_view_angles(cmd->m_view_angles);
	}
}

void c_projectile_aim::draw_path() {
	if (!config->m_projectile_aim.m_draw_projectile_path || m_positions.empty() || (ctx->m_engine_client->time() > m_time_stamp + 5.0f))
		return;

	const auto& start = m_positions.front();

	for (auto i = 1u; i < m_positions.size(); i++) {
		const auto& end = m_positions[i];

		auto start_screen = vector(), end_screen = vector();

		if (!tf2::world_to_screen(start, start_screen) || !tf2::world_to_screen(end, end_screen))
			continue;

		color line_color = color(255, 255, 255, 255);

		render->line(start_screen.m_x, start_screen.m_y, end_screen.m_x, end_screen.m_y, line_color);
	}
}

bool c_projectile_aim::find_ideal_target(c_base_player* player) {
	m_target = nullptr;

	auto shoot_position = player->get_shoot_position();
	auto angles = ctx->m_engine_client->get_view_angles();

	auto best_fov = std::numeric_limits< float >::max();

	for (const auto& aimbot_entity : ctx->aimbot_entities) {
		const auto& entity = aimbot_entity->as<c_base_player>();
		if (!entity || entity->is_dormant() || entity == ctx->m_local_player)
			continue;

		if (entity->is_player() && (entity->is_invulnerable() || entity->is_bonked() || entity->is_ghost()))
			continue;

		if (!entity->is_enemy() && ctx->m_local_weapon->get_weapon_id() != TF_WEAPON_CROSSBOW)
			continue;

		if (ctx->m_local_weapon->get_weapon_id() == TF_WEAPON_CROSSBOW && entity->is_building())
			continue;

		if (entity->is_cloaked() && config->m_aimbot_global.m_ignore_cloaked)
			continue;

		if (!(entity->is_player() && entity->as< c_base_player >()->is_alive()) && !entity->is_building())
			continue;

		if (entity->is_player() && entity->is_player_on_friend_list() && config->m_aimbot_global.m_ignore_friends)
			continue;

		auto fov_to = math::calculate_fov(angles, math::calculate_angle(shoot_position, entity->get_world_space_center()));

		if (fov_to > config->m_aimbot_global.m_aim_fov)
			continue;

		if (fov_to < best_fov) {
			best_fov = fov_to;
			m_target = entity;
		}
	}

	return (m_target != nullptr);
}

std::vector<vector> c_projectile_aim::get_aim_positions(c_base_combat_weapon* weapon) {
	if ((is_pipe(weapon) || is_rocket(weapon)) && (m_target->as<c_base_player>()->m_flags() & FL_ONGROUND & FL_DUCKING & FL_INWATER))
		return { m_target->get_world_space_center() - vector(0.0f, 0.0f, 27.0f), m_target->get_world_space_center(), m_target->as<c_base_player>()->get_hitbox_position(HITBOX_HEAD) };

	if (weapon->get_weapon_id() == TF_WEAPON_COMPOUND_BOW)
		return { m_target->as<c_base_player>()->get_hitbox_position(HITBOX_HEAD) };

	if (weapon->get_weapon_id() == TF_WEAPON_ROCKETLAUNCHER)
		return { m_target->as<c_base_player>()->get_hitbox_position(HITBOX_LEFT_FOOT), m_target->as<c_base_player>()->get_hitbox_position(HITBOX_RIGHT_FOOT) };

	if (weapon->get_weapon_id() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT)
		return { m_target->as<c_base_player>()->get_hitbox_position(HITBOX_RIGHT_FOOT), m_target->as<c_base_player>()->get_hitbox_position(HITBOX_LEFT_FOOT) };

	if (weapon->get_weapon_id() == TF_WEAPON_PIPEBOMBLAUNCHER)
		return { m_target->as<c_base_player>()->get_hitbox_position(HITBOX_CHEST), m_target->as<c_base_player>()->get_hitbox_position(HITBOX_PELVIS) };

	return { m_target->get_world_space_center(), m_target->as<c_base_player>()->get_hitbox_position(HITBOX_HEAD), m_target->get_world_space_center() - vector(0.0f, 0.0f, 27.0f) };
}

void c_projectile_aim::handle_player_aim(c_base_player* player, c_base_combat_weapon* weapon, c_user_cmd* cmd) {
	auto shoot_position = player->get_shoot_position();

	if (movement_sim->init(m_target->as<c_base_player>())) {
		auto simulated_position = vector();
		auto ok = false;

		float maxSimulationTime = config->m_projectile_aim.m_dynamic_sim ? (config->m_projectile_aim.m_max_simulation_time > 0 ? config->m_projectile_aim.m_max_simulation_time : 25.0f) : 25.0f;

		float enemySpeed = m_target->m_initial_velocity().length();

		float adjustedSimulationTime = maxSimulationTime + (config->m_projectile_aim.m_dynamic_sim ? (enemySpeed * 0.1f) : 0.0f);

		for (auto t = 0.0f; t < adjustedSimulationTime; t += TICK_INTERVAL) {
			simulated_position = movement_sim->run_tick();

			if (!solve_projectile(weapon, shoot_position, simulated_position))
				break;

			if (m_time > adjustedSimulationTime)
				break;

			if (m_projectile_info.m_type == TF_PROJECTILE_BALLOFFIRE && m_time > 0.1753f)
				break;

			if (t > m_time + get_latency()) {
				ok = true;
				break;
			}
		}

		movement_sim->restore();

		if (ok) {
			auto ok_ = false;

			for (const auto& position : get_aim_positions(weapon)) {
				auto simulated_position_ = simulated_position + (position - m_target->m_origin());

				if (!solve_projectile(weapon, shoot_position, simulated_position_))
					continue;

				float maxTime = config->m_projectile_aim.m_dynamic_sim ? adjustedSimulationTime : 25.0f;

				if (m_time > maxTime)
					continue;

				if (m_projectile_info.m_type == TF_PROJECTILE_BALLOFFIRE && m_time > 0.1753f)
					continue;

				solve_pipe_aim(weapon);

				if (!trace_projectile_path(player, weapon, simulated_position_))
					continue;

				aim(weapon, cmd);
				ok_ = true;
				break;
			}

			if (!ok_) {
				m_time_stamp = 0.0f;
				m_positions.clear();
			}
		}
	}
}

void c_projectile_aim::handle_building_aim(c_base_player* player, c_base_combat_weapon* weapon, c_user_cmd* cmd) {
	auto shoot_position = player->get_shoot_position();
	auto position = m_target->get_world_space_center();

	if (!solve_projectile(weapon, shoot_position, position))
		return;

	solve_pipe_aim(weapon);

	if (!trace_projectile_path(player, weapon, m_target->m_origin()))
		return;

	aim(weapon, cmd);
}

float c_projectile_aim::get_latency() {
	if (auto net_channel = ctx->m_engine_client->get_net_channel_info())
		return (net_channel->get_latency(0));

	return 0.0f;
}

bool c_projectile_aim::is_pipe(c_base_combat_weapon* weapon) {
	switch (weapon->get_weapon_id()) {
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_PIPEBOMBLAUNCHER: return true;
	default: return false;
	}
}

bool c_projectile_aim::is_rocket(c_base_combat_weapon* weapon) {
	switch (weapon->get_weapon_id()) {
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT: return true;
	default: return false;
	}
}

float c_projectile_aim::solve_projectile_speed(c_base_combat_weapon* weapon, const vector& a, const vector& b) {
	if (weapon->get_weapon_id() != TF_WEAPON_GRENADELAUNCHER && weapon->get_weapon_id() != TF_WEAPON_PIPEBOMBLAUNCHER)
		return m_projectile_info.m_speed;

	const auto projectile_speed = m_projectile_info.m_speed;
	const auto projectile_gravity = 800.0f * m_projectile_info.m_gravity_modifier;

	const auto delta = (b - a);
	const auto delta_length = delta.length_2d();

	const auto root = powf(projectile_speed, 4.0f) - projectile_gravity * (projectile_gravity * powf(delta_length, 2.0f) + 2.0f * delta.m_z * powf(projectile_speed, 2.0f));

	if (root < 0.0f)
		return 0.0f;

	const auto pitch = atanf((powf(m_projectile_info.m_speed, 2.0f) - sqrtf(root)) / (projectile_gravity * (delta_length)));
	const auto time = delta_length / (cosf(pitch) * projectile_speed);

	auto drag_force = 1.0f;

	switch (weapon->m_item_definition_index()) {
	case Demoman_m_GrenadeLauncher:
	case Demoman_m_GrenadeLauncherR:
	case Demoman_m_FestiveGrenadeLauncher:
	case Demoman_m_TheIronBomber:
	case Demoman_m_Autumn:
	case Demoman_m_MacabreWeb:
	case Demoman_m_Rainbow:
	case Demoman_m_SweetDreams:
	case Demoman_m_CoffinNail:
	case Demoman_m_TopShelf:
	case Demoman_m_Warhawk:
	case Demoman_m_ButcherBird: {
		drag_force = 0.16f;
		break;
	}

	case Demoman_m_TheLochnLoad: {
		drag_force = 0.048f;
		break;
	}

	case Demoman_m_TheLooseCannon: {
		drag_force = 0.49f;
		break;
	}

	case Demoman_s_StickybombLauncher:
	case Demoman_s_StickybombLauncherR:
	case Demoman_s_FestiveStickybombLauncher:
	case Demoman_s_TheQuickiebombLauncher:
	case Demoman_s_TheScottishResistance: {
		drag_force = 0.2f;
		break;
	}
	}

	return projectile_speed - (projectile_speed * time) * drag_force;
}

void c_projectile_aim::solve_pipe_aim(c_base_combat_weapon* weapon) {
	if (is_pipe(weapon)) {
		auto forward = vector(), up = vector();
		math::angle_vectors(vector(-RAD_TO_DEG(m_angle.m_x), RAD_TO_DEG(m_angle.m_y)), &forward, nullptr, &up);

		auto velocity = (forward * m_projectile_info.m_speed) - (up * 200.0f);
		auto angle = vector();
		math::vector_angles(velocity, angle);

		m_angle.m_x = -DEG_TO_RAD(angle.m_x);
	}
}

bool c_projectile_aim::trace_projectile_path(c_base_player* player, c_base_combat_weapon* weapon, vector pos) {
	auto view_angles = vector(-RAD_TO_DEG(m_angle.m_x), RAD_TO_DEG(m_angle.m_y));
	if (!projectile_sim->get_info(player, weapon, view_angles, m_projectile_info) || !projectile_sim->init(m_projectile_info)) {
		return false;
	}

	m_positions.clear();

	for (auto time = 0.0f; time < 5.0f; time += TICK_INTERVAL) {
		auto start = projectile_sim->get_origin();
		projectile_sim->run_tick();
		auto end = projectile_sim->get_origin();

		c_game_trace trace;
		c_trace_filter_hitscan filter;
		ray_t ray;
		filter.m_skip = player;

		vector ray_min = (weapon->get_weapon_id() == TF_WEAPON_COMPOUND_BOW) ? vector(-8.0f, -8.0f, -8.0f) : vector(-4.0f, -4.0f, -4.0f);
		vector ray_max = (weapon->get_weapon_id() == TF_WEAPON_COMPOUND_BOW) ? vector(8.0f, 8.0f, 8.0f) : vector(4.0f, 4.0f, 4.0f);

		ray.init(start, end, ray_min, ray_max);
		ctx->m_engine_trace->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);

		if (trace.did_hit()) {

			if (!(tf2::is_visible(nullptr, m_target, start, m_target->get_world_space_center(), MASK_SHOT | CONTENTS_GRATE, ray_min, ray_max) && (start - pos).length() < 140)) {
				return false;
			}
			else {
				return true;
			}
		}

		if (tf2::is_visible(nullptr, m_target, start, m_target->get_world_space_center(), MASK_SHOT | CONTENTS_GRATE, ray_min, ray_max) && (start - pos).length() < 140) {
			return true;
		}

		m_time_stamp = ctx->m_engine_client->time();
		m_positions.push_back(projectile_sim->get_origin());
	}

	return true;
}

bool c_projectile_aim::solve_projectile(c_base_combat_weapon* weapon, const vector& start, const vector& end) {
	const vector delta = end - start;
	const float delta_length = delta.length_2d();

	const float projectile_speed = solve_projectile_speed(weapon, start, end);
	const float projectile_gravity = (is_pipe(weapon)) ? 800.0f * m_projectile_info.m_gravity_modifier : ctx->m_cvar->find_var("sv_gravity")->get_float() * m_projectile_info.m_gravity_modifier;

	if (projectile_gravity) {
		const float root = powf(projectile_speed, 4.0f) - projectile_gravity * (projectile_gravity * powf(delta_length, 2.0f) + 2.0f * delta.m_z * powf(projectile_speed, 2.0f));

		if (root < 0.0f)
			return false;

		m_angle.m_x = atanf((powf(projectile_speed, 2.0f) - sqrtf(root)) / (projectile_gravity * delta_length));
		m_angle.m_y = atan2f(delta.m_y, delta.m_x);
	}
	else {
		const vector angle_to = math::calculate_angle(start, end);
		m_angle.m_x = -DEG_TO_RAD(angle_to.m_x);
		m_angle.m_y = DEG_TO_RAD(angle_to.m_y);
	}

	m_time = delta_length / (cosf(m_angle.m_x) * projectile_speed);
	return true;
}

bool c_projectile_aim::SolveProjectile(c_base_combat_weapon* weapon, const vector& start, const vector& point, float& solvedTime) {
	projectile_info_t projectileInfo;

	if (!projectile_sim->get_info(ctx->m_local_player, weapon, point, projectileInfo))
		return false;

	solvedTime = projectile_sim->init(projectileInfo);

	return true;
}

// IDK WHY NO WORK
/*bool c_projectile_aim::HandleSplashBot(c_base_player* target, const vector& start, const vector& position, float& timeOut) {
	if (!config->m_projectile_aim.m_splash_bot)
		return false;

	const int weaponId = ctx->m_local_weapon->get_weapon_id();
	if (weaponId != TF_WEAPON_ROCKETLAUNCHER)
		return false;

	const int numSamples = 256;
	const float radius = config->m_projectile_aim.m_splash_radius;

	std::vector<vector> spherePoints = ComputeSphere(position, radius, numSamples);

	std::sort(spherePoints.begin(), spherePoints.end(), [&](const vector& a, const vector& b) -> bool {
		return (a + position).length_2d() < (b + position).length_2d();
		});

	for (const auto& spherePoint : spherePoints) {
		const vector point = spherePoint + position;

		float solvedTime;
		if (!SolveProjectile(ctx->m_local_weapon, start, point, solvedTime))
			continue;

		if (solvedTime >= 0.0f && solvedTime <= 5.0f) {
			float angleOut = -RAD_TO_DEG(std::atan2(point.m_y - start.m_y, point.m_x - start.m_x));
			ctx->m_cvar->console_color_printf({ 255, 55, 55, 255 }, "SPLASHBOT");
			ctx->m_cvar->console_color_printf({ 55, 255, 55, 255 }, "Target: %s (%s)", target->get_client_class(), target->get_shoot_position());
			timeOut = solvedTime;

			return true;
		}
	}

	return false;
}

bool c_projectile_aim::IsRocketLauncher(int weaponId) {
	return weaponId == TF_WEAPON_ROCKETLAUNCHER;
}

std::vector<vector> c_projectile_aim::ComputeSphere(const vector& center, float radius, int numSamples) {
	std::vector<vector> points;
	points.reserve(numSamples);

	const float GoldenAngle = 2.39996323f;

	for (int i = 0; i < numSamples; ++i) {
		const float Inclination = acosf(1.0f - 2.0f * i / numSamples);
		const float Azimuth = GoldenAngle * i;

		const vector sample = vector(cosf(Azimuth) * sinf(Inclination), sinf(Azimuth) * sinf(Inclination), cosf(Inclination)) * radius;

		points.push_back(sample);
	}

	return points;
}*/

void c_projectile_aim::aim(c_base_combat_weapon* weapon, c_user_cmd* cmd) {
	if (config->m_projectile_aim.m_auto_shoot && config->m_aimbot_global.m_aim_key.is_held())
		cmd->m_buttons |= IN_ATTACK;

	if (!ctx->m_local_weapon->can_shoot())
		return;

	if (cmd->m_buttons & IN_ATTACK) {
		cmd->m_view_angles = vector(-RAD_TO_DEG(m_angle.m_x), RAD_TO_DEG(m_angle.m_y));

		ctx->m_aimbot_angles = cmd->m_view_angles;

		ctx->m_aimbot_delay = 0;

		switch (weapon->get_weapon_id()) {
		case TF_WEAPON_PIPEBOMBLAUNCHER:
			ctx->m_aimbot_delay = 80;
			break;
		case TF_WEAPON_GRENADELAUNCHER:
			ctx->m_aimbot_delay = 80;
			break;
		case TF_WEAPON_COMPOUND_BOW:
			ctx->m_aimbot_delay = 80;
			break;
		case TF_WEAPON_ROCKETLAUNCHER:
			ctx->m_aimbot_delay = 80;
			break;
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			ctx->m_aimbot_delay = 80;
			break;

		default:
			break;
		}

		if ((weapon->get_weapon_id() == TF_WEAPON_PIPEBOMBLAUNCHER || weapon->get_weapon_id() == TF_WEAPON_COMPOUND_BOW) &&
			weapon->m_charge_begin_time() > 0.0f)
			cmd->m_buttons &= ~IN_ATTACK;
	}
}
