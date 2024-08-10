#include "visuals.h"
#include "../../include/render.h"
#include "../menu/menu.h"
#include "../../include/tf2.h"
#include "../projectile_aim/projectile_aim.h"
#include <deque>
#include "../seed_prediction/seed_prediction.h"
#include "../crithack/crithack.h"

static std::string m_vote_target = "temp";
static std::string m_vote_caller = "temp";

int calculateNextCritCost(c_base_combat_weapon* weapon) {
	if (weapon->get_slot() != e_weapon_slots::SLOT_MELEE) {
		int critMultiplier = 3;
		int weaponDamage = weapon->get_weapon_data().m_damage;
		int bulletsPerShot = weapon->get_weapon_data().m_bullets_per_shot;
		int cumBucket = weapon->m_crit_token_bucket();
		int baseDamage = (1000 - weaponDamage);

		int fullDamage = weaponDamage * bulletsPerShot;
		int criticalFullDamage = fullDamage * critMultiplier;

		int bucketCritsMax = baseDamage / (criticalFullDamage + 1);
		int bucketCrits = (cumBucket - weaponDamage) / criticalFullDamage;

		return criticalFullDamage;
	}
	else {

		return 0;
	}
}

void c_visuals::draw_indicators() {
	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon)
		return;

	if (ctx->m_engine_vgui->is_game_ui_visible())
		return;

	auto indicator_amount = 0;

	if (config->m_exploits.m_doubletap_indicator)
		indicator_amount++;

#ifndef JW_RELEASE_BUILD
	if (config->m_seed_pred.m_draw_indicator)
		indicator_amount++;
#endif

	auto height_to_deduct = (indicator_amount * (ctx->m_tahoma_narrow.m_tall + 20)) * 0.5f;

	ctx->m_surface->draw_set_alpha(0.7f); {
		auto y_offset = ctx->m_screen_height * 0.5f - height_to_deduct;

		if (config->m_exploits.m_doubletap_indicator) {
			int ticks = ctx->m_charged_ticks <= 0 ? ctx->m_client_state->m_choked_commands : ctx->m_charged_ticks;
			float max_ticks = 20.f;

			std::string dt_status;
			color dt_color(255, 255, 255);

			if (ticks <= 0) {
				dt_status = "LOW CHARGE";
				dt_color = color(255, 0, 0);
			}
			else if (ticks < max_ticks) {
				dt_status = "CHARGING";
				dt_color = color(255, 165, 0);
			}
			else {
				dt_status = "CHARGED";
				dt_color = color(0, 255, 0);
			}

			render->outlined_text(50, 450, e_string_align::ALIGN_DEFAULT, ctx->m_tahoma_narrow.m_font, dt_color, "  Ticks %i     %s", ticks, dt_status.c_str());
			y_offset += ctx->m_tahoma_narrow.m_tall;

			if (config->m_misc.m_reduce_input_delay)
				max_ticks = 20.f;

			int bar_width = math::remap_val_clamped(ticks, 1.0f, max_ticks, 1.0f, 136.0f);

			render->gradient_rect(55, 465, 136, 12, color(20, 20, 20), color(20, 20, 20), true);

			render->gradient_rect(55, 465, bar_width, 12, color(61, 191, 227), color(61, 191, 227), true);

			if (ticks < max_ticks) {
				render->outlined_rect(54, 464, 138, 14, color(255, 0, 0));
			}
			else {
				render->outlined_rect(54, 464, 138, 14, color(0, 255, 0));
			}

			y_offset += 10;
		}

		if (config->m_exploits.m_crithack_indicator && ctx->m_local_weapon->get_slot() <= 2) {
			con_var* crits_enabled = ctx->m_cvar->find_var("tf_weapon_criticals");

			if (!crits_enabled->get_int() ||
				(ctx->m_local_player->m_player_class() == CLASS_SPY && ctx->m_local_weapon->get_slot() > 0) ||
				(ctx->m_local_player->m_player_class() == CLASS_SNIPER && ctx->m_local_weapon->get_slot() == 0) ||
				(ctx->m_local_weapon->get_weapon_id() == TF_WEAPON_LUNCHBOX) ||
				tf2::is_non_aimable(ctx->m_local_weapon)) {

				render->outlined_text(ctx->m_screen_width * 0.5f, ctx->m_screen_height * 0.5f + 50,
					e_string_align::ALIGN_CENTER, ctx->m_tahoma_narrow.m_font, color(255, 255, 255), "");
			}
			else {
				const int indicatorX = 70;
				const int indicatorY = 175;
				const int boxWidth = 65;
				const int boxHeight = 10;

				int chargeBarWidth = static_cast<int>(boxWidth * (static_cast<float>(crithack->m_bucket_crits) / crithack->m_bucket_crits_max));

				render->filled_rect(indicatorX - boxWidth / 2, indicatorY, boxWidth, boxHeight, color(180, 180, 180, 255));
				render->outlined_rect(indicatorX - boxWidth / 2 - 1, indicatorY - 1, boxWidth + 2, boxHeight + 2, color(0, 0, 0, 255));

				render->filled_rect(indicatorX - boxWidth / 2, indicatorY, chargeBarWidth, boxHeight, color(255, 140, 0, 255));

				render->outlined_text(indicatorX, indicatorY + boxHeight + 5, e_string_align::ALIGN_CENTER,
					ctx->m_tahoma_narrow.m_font, color(255, 255, 255), "%i/%i crits", crithack->m_bucket_crits, crithack->m_bucket_crits_max);

				int predictedCritDmg = static_cast<int>(crithack->m_official_crit_dmg * (static_cast<float>(crithack->m_bucket_crits) / crithack->m_bucket_crits_max));
				render->outlined_text(indicatorX, indicatorY + boxHeight + 20, e_string_align::ALIGN_CENTER,
					ctx->m_tahoma_narrow.m_font, color(255, 255, 255), "official crit damage: %i", predictedCritDmg);

				if (crithack->m_bucket_crits > 0) {
					int nextCritCost = calculateNextCritCost(ctx->m_local_weapon);
					render->outlined_text(indicatorX, indicatorY + boxHeight + 35, e_string_align::ALIGN_CENTER,
						ctx->m_tahoma_narrow.m_font, color(255, 255, 255), "next crit cost: %i", nextCritCost);
				}
			}
		}

		// this is pasted from travishook 

		if (config->m_seed_pred.m_draw_indicator) {
			std::string text = "";
			int textHeight = 20;

			if (ctx->m_local_weapon->is_spread_weapon()) {
				switch (seed_prediction->m_no_spread_synced) {
				case nospread_sync_state_t::NOT_SYNCED:
					text = "Not Synced!";
					break;
				case nospread_sync_state_t::CORRECTING:
					text = "Correcting...";
					break;
				case nospread_sync_state_t::SYNCED:
					text = "Synced!";
					break;
				case nospread_sync_state_t::DEAD_SYNC:
					text = "Dead Sync!";
					break;
				}
			}
			else {
				text = "Disabled!";
			}

			int indicatorX = 10;
			int indicatorY = (ctx->m_screen_height - (textHeight * 2 + 10)) / 2;

			render->outlined_text(indicatorX, indicatorY, e_string_align::ALIGN_DEFAULT, ctx->m_verdana_narrow.m_font, color(255, 200, 0), text.c_str());
			render->outlined_text(indicatorX, indicatorY + textHeight, e_string_align::ALIGN_DEFAULT, ctx->m_verdana_narrow.m_font, color(255, 255, 255), "Step %.0f", seed_prediction->m_mantissa_step);

			y_offset = std::fmax(y_offset, indicatorY + textHeight * 2 + 10);
		}

		if (config->m_visuals.m_spectatorlist && ctx->m_local_player && ctx->m_local_player->is_alive()) {
			int numSpectators = 0;

			for (const auto& player : ctx->players) {
				if (numSpectators >= 6 || !player || player->is_alive() || player->is_dormant() || player == ctx->m_local_player)
					continue;

				const auto& observed = ctx->m_entity_list->get_client_entity_from_handle(player->ObserverTarget());
				if (observed && observed == ctx->m_local_player) {
					player_info_t pi;
					if (ctx->m_engine_client->get_player_info(player->entindex(), &pi) && pi.m_name) {
						numSpectators++;
					}
				}
			}

			if (numSpectators > 0) {
				float padding = 10.0f;

				const wchar_t* titleText = L"Spectator list";
				float titleTextWidth = ctx->m_tahoma_narrow.m_font * wcslen(titleText);

				float textWidth = titleTextWidth;

				for (const auto& player : ctx->players) {
					if (numSpectators >= 6 || !player || player->is_alive() || player->is_dormant() || player == ctx->m_local_player)
						continue;

					const auto& observed = ctx->m_entity_list->get_client_entity_from_handle(player->ObserverTarget());
					if (observed && observed == ctx->m_local_player) {
						player_info_t pi;
						if (ctx->m_engine_client->get_player_info(player->entindex(), &pi) && pi.m_name) {
							const wchar_t* playerName = util::utf_to_wide(pi.m_name).c_str();
							float playerNameWidth = ctx->m_tahoma_bold.m_tall * wcslen(playerName);
							textWidth = std::max(textWidth, playerNameWidth + padding);
						}
					}
				}

				textWidth += padding;

				render->outlined_text(ctx->m_screen_width * 0.5f, ctx->m_screen_height * 0.1f + 5, e_string_align::ALIGN_CENTER, ctx->m_tahoma_bold.m_font, color(61, 191, 227), L"Spectating You");

				float totalHeight = 15;

				for (const auto& player : ctx->players) {
					if (numSpectators >= 6 || !player || player->is_alive() || player->is_dormant() || player == ctx->m_local_player)
						continue;

					const auto& observed = ctx->m_entity_list->get_client_entity_from_handle(player->ObserverTarget());
					if (observed && observed == ctx->m_local_player) {
						color stringColor = player->is_player_on_friend_list() ? color(150, 255, 150) : color(255, 255, 255);

						std::wstring spectatorMode;
						if (player->ObserverMode() == OBS_MODE_FIRSTPERSON) {
							spectatorMode = L"[ 1st ] ";
							stringColor = color(255, 50, 50);
						}
						else if (player->ObserverMode() == OBS_MODE_THIRDPERSON) {
							spectatorMode = L"[ 3rd ] ";
						}
						else if (player->ObserverMode() == OBS_MODE_FREEZECAM) {
							spectatorMode = L"[ Freeze ] ";
							stringColor = color(255, 50, 50);
						}

						player_info_t pi;
						if (ctx->m_engine_client->get_player_info(player->entindex(), &pi) && pi.m_name) {
							const wchar_t* playerName = util::utf_to_wide(pi.m_name).c_str();
							if (playerName) {
								std::wstring displayText = spectatorMode + playerName;

								float y_position = ctx->m_screen_height * 0.1f + totalHeight;
								render->outlined_text(ctx->m_screen_width * 0.5f, y_position, e_string_align::ALIGN_CENTER, ctx->m_tahoma_bold.m_font, stringColor, displayText.c_str());
								totalHeight += ctx->m_tahoma_bold.m_tall;
							}
						}
					}
				}
			}

			if (config->m_misc.m_vote_revealer && m_vote_in_progress) {
				render->outlined_text(37, y_offset, e_string_align::ALIGN_DEFAULT, ctx->m_tahoma_narrow.m_font, config->m_colors.m_ui_text, "Vote_Kick called on %s by %s", m_vote_target.c_str(), m_vote_caller.c_str());
				render->outlined_text(37, y_offset + 12, e_string_align::ALIGN_DEFAULT, ctx->m_tahoma_narrow.m_font, config->m_colors.m_ui_text, "Yes = %i  |  No = %i", m_vote_yes, m_vote_no);
			}

			if (config->m_hvh.m_anti_aim.m_rYaw_master) {
				indicator_amount++;
				int distance = 50;

				const auto& vOrigin = ctx->m_local_player->get_abs_angles();
				vector vScreen1, vScreen2;

				vScreen1 = math::calculate_angle(vector(0, 0, 0), vector(vOrigin.m_x, vOrigin.m_y, 0));
				vScreen2 = math::calculate_angle(vector(0, 0, 0), vector(vOrigin.m_x, ctx->m_real_angles.m_y, 0));

				render->line(vScreen1.m_x, vScreen1.m_y, vScreen2.m_x, vScreen2.m_y, color(0, 255, 0));
			}

			if (config->m_hvh.m_anti_aim.m_fYaw_master) {
				indicator_amount++;
				int distance = 50;

				const auto& vOrigin = ctx->m_local_player->get_abs_angles();
				vector vScreen1, vScreen2;

				vScreen1 = math::calculate_angle(vector(0, 0, 0), vector(vOrigin.m_x, vOrigin.m_y, 0));
				vScreen2 = math::calculate_angle(vector(0, 0, 0), vector(vOrigin.m_x, ctx->m_fake_angles.m_y, 0));

				render->line(vScreen1.m_x, vScreen1.m_y, vScreen2.m_x, vScreen2.m_y, color(255, 0, 0));
			}


		} ctx->m_surface->draw_set_alpha(1.0f);
	}
}

void c_visuals::draw_aimbot_fov()
{
	if (!ctx->m_local_weapon || !config->m_aimbot.m_enabled || !config->m_aimbot.m_draw_fov || ctx->m_engine_vgui->is_game_ui_visible())
		return;

	float aimbot_fov = config->m_aimbot_global.m_aim_fov;
	float camera_fov = config->m_visuals.m_field_of_view;
	float r = tanf(DEG_TO_RAD(aimbot_fov * 0.5f)) / tanf(DEG_TO_RAD(camera_fov * 0.5f)) * ctx->m_screen_width * 0.5f;

	render->draw_circle(ctx->m_screen_width * 0.5f, ctx->m_screen_height * 0.5f, r, 100, config->m_colors.m_aimbot_fov);
}

void c_visuals::trace_projectile_path() {
	if (!ctx->m_local_player || !ctx->m_local_player->is_alive() || !ctx->m_local_weapon || !config->m_projectile_aim.m_projectile_path_any_angle)
		return;

	if (ctx->m_engine_vgui->is_game_ui_visible())
		return;

	projectile_info_t info = { };

	if (!projectile_sim->get_info(ctx->m_local_player, ctx->m_local_weapon, ctx->m_engine_client->get_view_angles(), info))
		return;

	if (config->m_automations.m_charge_max > 0 && ctx->m_local_weapon->get_slot() == 1 && ctx->m_local_player->m_player_class() == CLASS_DEMOMAN) {
		//modify speed based on custom charge max
		float get_max_charge = config->m_automations.m_charge_max / 27.0f;
		if (ctx->m_local_weapon->m_item_definition_index() == Demoman_s_TheQuickiebombLauncher)
			get_max_charge = config->m_automations.m_charge_max / 70.0f;

		float diff = 15;
		if (ctx->m_local_weapon->m_item_definition_index() == Demoman_s_TheQuickiebombLauncher)
			diff = 5;

		float new_speed = math::remap_val_clamped(get_max_charge - TICKS_TO_TIME(diff), 0.0f, tf2::attrib_hook_float(4.0f, "stickybomb_charge_rate", ctx->m_local_weapon), 900.0f, 2400.0f);
		info.m_speed = new_speed;
	}

	if (!projectile_sim->init(info))
		return;

	for (int n = 0; n < TIME_TO_TICKS(10.0f); n++) {
		auto old_position = projectile_sim->get_origin();
		projectile_sim->run_tick();
		auto new_position = projectile_sim->get_origin();

		static auto render_line = util::find_signature< void(__cdecl*)(const vector&, const vector&, color, bool) >("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B F0 85 F6");
		render_line(old_position, new_position, color(235, 235, 235, 175), false);

		ray_t ray = { };
		ray.init(old_position, new_position, vector(-4.5f, -4.5f, -4.5f), vector(4.5f, 4.5f, 4.5f));

		c_trace_filter_hitscan filter = { };
		filter.m_skip = ctx->m_local_player;

		trace_t trace = { };
		ctx->m_engine_trace->trace_ray(ray, MASK_SHOT, &filter, &trace);

		if (trace.did_hit()) {
			ctx->m_projectile_camera_position = new_position;
			auto angles = vector();
			math::vector_angles(trace.m_plane.m_normal, angles);

			static auto render_box = util::find_signature< void(__cdecl*)(const vector&, const vector&, const vector&, const vector&, color, bool, bool) >("engine.dll", "55 8B EC 51 8B 45 ? 8B C8 FF 75");
			render_box(new_position, angles, vector(-1.0f, -4.5f, -4.5f), vector(1.0f, 4.5f, 4.5f), color(195, 35, 35, 200), false, false);

			static auto render_wireframe_box = util::find_signature< void(__cdecl*)(const vector&, const vector&, const vector&, const vector&, color, bool) >("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B F0 89 75 ? 85 F6 74 ? 8B 06 8B CE FF 50 ? A1");
			render_wireframe_box(new_position, angles, vector(-1.0f, -4.5f, -4.5f), vector(1.0f, 4.5f, 4.5f), color(235, 65, 65, 235), false);

			break;
		}
	}

}

void c_visuals::visualize_stickies_path() {
	if (!ctx->m_local_player || !config->m_esp.m_predict_stickies || ctx->m_engine_vgui->is_game_ui_visible())
		return;

	if (ctx->m_engine_vgui->is_game_ui_visible())
		return;

	for (const auto& entity : ctx->projectiles) {
		if (!entity || !entity->is_projectile() || !entity->get_model() || entity->get_client_class()->m_class_id != class_id_t::CTFGrenadePipebombProjectile)
			continue;

		projectile_info_t m_projectile_info;
		vector angle, startscr, endscr;
		m_projectile_info.m_speed = entity->estimate_abs_velocity().length_2d();
		if (entity->estimate_abs_velocity().length() <= 1.f)
			continue;

		const float g = 800;

		math::vector_angles(entity->estimate_abs_velocity(), angle);

		vector alongDir = entity->estimate_abs_velocity();
		alongDir.normalize_in_place();

		vector start = entity->m_origin(), end = entity->m_origin();

		float alongVel = sqrt(entity->estimate_abs_velocity().m_x * entity->estimate_abs_velocity().m_x + entity->estimate_abs_velocity().m_y * entity->estimate_abs_velocity().m_y);

		for (auto t = 0.0f; t < 5.0f; t += TICK_INTERVAL) {
			float along = alongVel * t;
			float height = entity->estimate_abs_velocity().m_z * t - 0.5f * g * t * t;

			start = end;
			end.m_x = entity->m_origin().m_x + alongDir.m_x * along;
			end.m_y = entity->m_origin().m_y + alongDir.m_y * along;
			end.m_z = entity->m_origin().m_z + height;

			tf2::world_to_screen(start, startscr);
			tf2::world_to_screen(end, endscr);
			render->line(startscr.m_x, startscr.m_y, endscr.m_x, endscr.m_y, config->m_colors.m_ui_accent);

			if (!tf2::is_visible(entity, NULL, start, end, MASK_SHOT)) {
				ctx->m_debug_overlay->add_box_overlay(end, entity->m_mins(), entity->m_maxs(), vector(), config->m_colors.m_ui_accent.m_r, config->m_colors.m_ui_accent.m_g, config->m_colors.m_ui_accent.m_b, 100, 0.03f);
				break;
			}
		}
	}
}

void c_visuals::draw_hitboxes(c_base_player* player, const model_t* model, matrix_3x4* bones, const color& colorface, const color& coloredge, float time) {
	if (!model || !player || !bones)
		return;

	if (ctx->m_engine_vgui->is_game_ui_visible())
		return;

	const studiohdr_t* hdr = ctx->m_model_info->get_studio_model(model);
	const mstudiohitboxset_t* set = hdr->hitbox_set(player->m_hitbox_set());

	for (int i = 0; i < set->numhitboxes; ++i) {
		const mstudiobbox_t* bbox = set->hitbox(i);
		if (!bbox)
			continue;

		matrix_3x4 rotMatrix;
		math::angle_matrix(bbox->angle, rotMatrix);

		matrix_3x4 matrix;
		math::concat_transforms(bones[bbox->bone], rotMatrix, matrix);

		auto bbox_angle = vector();
		math::matrix_angles(matrix, bbox_angle);

		auto matrix_origin = vector();
		math::get_matrix_origin(matrix, matrix_origin);

		ctx->m_debug_overlay->add_box_overlay_2(matrix_origin, bbox->bbmin, bbox->bbmax, bbox_angle, colorface, coloredge, time);
	}
}

void c_visuals::draw_server_hitboxes() {
	static int old_tick = ctx->m_globals->m_tick_count;
	if (old_tick == ctx->m_globals->m_tick_count)
		return;
	old_tick = ctx->m_globals->m_tick_count;

	if (ctx->m_input->cam_is_third_person()) {
		static auto get_server_animating = reinterpret_cast<void* (*)(int)>(util::find_signature< uintptr_t >("server.dll", "55 8B EC 8B 55 ? 85 D2 7E ? A1"));
		static auto draw_server_hitboxes = reinterpret_cast<void(__thiscall*)(void*, float, bool)>(util::find_signature< uintptr_t >("server.dll", "55 8B EC 83 EC ? 57 8B F9 80 BF ? ? ? ? ? 0F 85 ? ? ? ? 83 BF ? ? ? ? ? 75 ? E8 ? ? ? ? 85 C0 74 ? 8B CF E8 ? ? ? ? 8B 97"));

		if (ctx->m_local_player && ctx->m_local_player->is_alive()) {
			void* server_animating = get_server_animating(ctx->m_local_player->entindex());
			if (server_animating)
				draw_server_hitboxes(server_animating, ctx->m_globals->m_interval_per_tick, true);
		}
	}
}

/*

	i hate chat vote revealers
	clutters the chat too much

*/

void c_visuals::vote_revealer_register_event(c_game_event* event) {
	if (!config->m_misc.m_vote_revealer)
		return;

	if (fnv::hash(event->get_name()) != fnv::hash("vote_cast"))
		return;

	(event->get_int("vote_option") == 0) ? m_vote_yes++ : m_vote_no++;
}

void c_visuals::vote_revealer_register_message(int message, bf_read& data) {
	if (!config->m_misc.m_vote_revealer || !ctx->m_local_player)
		return;

	if (message == VoteStart) {
		int team = data.ReadByte();
		int vote_id = data.ReadLong();
		int caller = data.ReadByte();
		char reason[64], name[64];
		data.ReadString(reason, 64);
		data.ReadString(name, 64);
		auto target = static_cast<uint8_t>(data.ReadByte());
		target >>= 1;

		if (team == ctx->m_local_player->m_team_num())
			return;

		m_vote_in_progress = true;
		m_vote_no = 1;
		m_vote_yes = 1;

		player_info_t caller_info, target_info;
		bool info = false;
		info = ctx->m_engine_client->get_player_info(caller, &caller_info) && ctx->m_engine_client->get_player_info(target, &target_info);

		//ctx->m_cvar->console_printf( "Team: %i  Target: %i  %s\n", team, target, target_info.m_name ); // this is correct
		//ctx->m_cvar->console_printf( "Team: %i  caller: %i  %s\n", team, caller, caller_info.m_name );

		m_vote_caller = caller_info.m_name;
		m_vote_target = target_info.m_name;
	}

	else m_vote_in_progress = false;
}

void c_visuals::draw_camera_outline() {
	if (ctx->m_engine_vgui->is_game_ui_visible())
		return;

	if (ctx->m_local_weapon && (ctx->m_local_weapon->get_weapon_id() == TF_WEAPON_PIPEBOMBLAUNCHER || ctx->m_local_weapon->get_weapon_id() == TF_WEAPON_SYRINGEGUN_MEDIC) && config->m_projectile_aim.m_projectile_path_any_angle && config->m_visuals.m_projectile_camera && !ctx->m_projectile_camera_position.is_zero()) {
		render->outlined_rect(visuals->camera_position.m_x,
			visuals->camera_position.m_y,
			visuals->camera_position.m_w,
			visuals->camera_position.m_h,
			config->m_colors.m_ui_accent);

		render->outlined_rect(visuals->camera_position.m_x - 1,
			visuals->camera_position.m_y - 1,
			visuals->camera_position.m_w + 2,
			visuals->camera_position.m_h + 2,
			config->m_colors.m_ui_background);
	}
}
