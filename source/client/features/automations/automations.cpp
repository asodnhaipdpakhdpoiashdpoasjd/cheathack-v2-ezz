#include "automations.h"
#include "../hitscan/hitscan.h"
#include "../game_events/game_events.h"

void c_automations::run( c_user_cmd* cmd ) {
	if ( !ctx->m_local_player || !ctx->m_local_player->is_alive( ) || !ctx->m_local_weapon || !config->m_automations.m_enabled)
		return;

	if ( config->m_automations.m_auto_detonate )
		auto_detonate( ctx->m_local_player, ctx->m_local_weapon, cmd );

	if (config->m_automations.m_auto_vaccinator)
		auto_vaccinator(cmd);
}

void c_automations::auto_detonate( c_base_player* player, c_base_combat_weapon* weapon, c_user_cmd* cmd ) {
	auto do_det = false;

	for ( const auto& sticky : ctx->projectiles ) {

		if ( sticky->get_client_class( )->m_class_id != class_id_t::CTFGrenadePipebombProjectile || sticky->m_pipebomb_type( ) != TYPE_STICKY )
			continue;

		if ( ctx->m_entity_list->get_client_entity_from_handle( sticky->m_thrower( ) ) != player )
			continue;

		c_base_entity* entity = nullptr;

		for ( c_entity_sphere_query sphere( sticky->get_world_space_center(), config->m_automations.m_auto_detonate_distance); ( entity = sphere.get_current_entity( ) ) != nullptr; sphere.next_entity( ) ) {
			if ( !entity || entity == player || entity->is_dormant( ) || ( !entity->is_player( ) && !entity->is_building( ) ) || ( entity->is_player( ) && !entity->as< c_base_player >( )->is_alive( ) ) || entity->m_team_num( ) == player->m_team_num( ) )
				continue;
			if (!tf2::is_visible(sticky, entity, sticky->get_world_space_center(), entity->get_world_space_center(), MASK_SHOT))
				continue;
			do_det = true;
		}

		if ( do_det )
			break;
	}

	if ( do_det )
		cmd->m_buttons |= IN_ATTACK2;
}

int c_automations::get_projectile_resistance_type(c_base_player* entity)
{
	if (!entity)
		return -1;

	if (!entity->is_projectile())
		return -1;

	if (entity->m_pipebomb_type() == TF_PROJECTILE_PIPEBOMB_REMOTE) // idk
		return internal_projectile_types::EXPLOSIVE;

	switch (entity->get_client_class()->m_class_id)
	{
	case class_id_t::CTFProjectile_Arrow:
	case class_id_t::CTFProjectile_Cleaver:
	case class_id_t::CTFProjectile_HealingBolt:
		return internal_projectile_types::HITSCAN;
	case class_id_t::CTFProjectile_Rocket:
	case class_id_t::CTFProjectile_SentryRocket:
	case class_id_t::CTFGrenadePipebombProjectile:
		return internal_projectile_types::EXPLOSIVE;
	case class_id_t::CTFProjectile_Flare:
	case class_id_t::CTFProjectile_BallOfFire:
	case class_id_t::CTFProjectile_SpellFireball:
		return internal_projectile_types::FIRE;
	}

	return -1;
}

void c_automations::gather_autovacc_data()
{
	if (!ctx->m_local_player || !ctx->m_local_weapon || !ctx->m_local_player->is_alive())
		return;

	if(!ctx->m_engine_client->get_net_channel_info())
		return;

	if (ctx->m_local_player->m_player_class() != CLASS_MEDIC)
		return;

	if (ctx->m_local_weapon->get_slot() != 1)
		return;

	if (!config->m_automations.m_auto_vaccinator)
		return;

	// cleanup first
	m_threats.clear();
	m_healing_targets.clear();

	for ( const auto& entity : ctx->players )
	{
		if ( !entity || entity->is_dormant( ) || entity == ctx->m_local_player || !entity->is_alive())
			continue;

		c_autovacc_threat threat = {};

		if ( entity->is_enemy( ) )
		{
			if ( entity->get_active_weapon( ) ) // ignore nonaimable weapons
				if ( !tf2::is_non_aimable( entity->get_active_weapon( ) ) )
					continue;

			if ( entity->m_player_class( ) == CLASS_SNIPER && entity->m_player_cond( ) & TFCond_Zoomed )
			{
				if ( tf2::is_visible( ctx->m_local_player, entity, ctx->m_local_player->get_shoot_position( ) + ctx->m_local_player->m_velocity( ) * TICKS_TO_TIME( 3 ), entity->get_world_space_center( ), MASK_SHOT | CONTENTS_GRATE )
					 && !( ctx->m_local_player->is_bullet_charged( ) ) )
				{
					threat.m_entity = entity;
					threat.m_projectile = false;
					threat.m_threat_scale = 999; // auto pop on here
					m_threats.emplace_back( threat );
					continue; // skip over good sir
				}

				if ( auto healing_target = ctx->m_entity_list->get_client_entity_from_handle( ctx->m_local_weapon->m_healing_target( ) )->as<c_base_player>( ) )
				{
					if ( healing_target->is_alive( ) )
					{
						if ( tf2::is_visible( healing_target, entity, healing_target->get_shoot_position( ) + healing_target->m_velocity( ) * TICKS_TO_TIME( 3 ), entity->get_world_space_center( ), MASK_SHOT | CONTENTS_GRATE )
							 && !( healing_target->is_bullet_charged( ) ) )
						{
							threat.m_entity = entity;
							threat.m_projectile = false;
							threat.m_threat_scale = 999;
							m_threats.emplace_back( threat );
							continue;
						}
					}
				}

			}
			threat.m_entity = entity;
			threat.m_projectile = false;
			threat.m_threat_scale = config->m_automations.m_auto_vaccinator_player_threat;
			if ( auto healing_target = ctx->m_entity_list->get_client_entity_from_handle( ctx->m_local_weapon->m_healing_target( ) )->as<c_base_player>( ) )
				threat.m_distance = healing_target->m_origin( ).dist_to( entity->m_origin( ) );
			else
				threat.m_distance = ctx->m_local_player->m_origin( ).dist_to( entity->m_origin( ) );

			if (threat.m_distance < 400.f && entity->m_player_class() == CLASS_HEAVY)
				threat.m_threat_scale = 100; // very scary and terrifing

			m_threats.emplace_back( threat );
			continue;
		}
		else
		{
			if ( entity->m_origin( ).dist_to( ctx->m_local_player->m_origin( ) ) >= 460.f )
				continue;

			if ( !tf2::is_visible( ctx->m_local_player, entity, ctx->m_local_player->get_shoot_position( ), entity->get_world_space_center( ), MASK_SHOT | CONTENTS_GRATE ) )
				continue;

			c_autovacc_healing healing = {};
			healing.m_entity = entity;
			healing.m_is_friend = entity->is_player_on_friend_list( );
			if ( healing.m_is_friend )
			{
				if ( entity->m_player_class( ) == CLASS_HEAVY )
					healing.m_priority = 10;

				if ( entity->m_player_class( ) == CLASS_SOLDIER || entity->m_player_class( ) == CLASS_DEMOMAN )
					healing.m_priority = 5;
			}
			else
			{
				healing.m_priority = 1;
			}

			m_healing_targets.emplace_back( healing );
			continue;
		}
	}

	for ( const auto entity : ctx->projectiles )
	{
		if ( entity && entity->is_enemy( ) )
		{
			c_autovacc_threat threat = {};
			if ( entity->m_pipebomb_type( ) != TF_PROJECTILE_PIPEBOMB_REMOTE && entity->estimate_abs_velocity( ).is_zero( ) )
				continue;

			threat.m_threat_scale = config->m_automations.m_auto_vaccinator_projectile_threat; // this is useless

			vector predicted_position = entity->get_world_space_center( ) + entity->estimate_abs_velocity( ) * TICKS_TO_TIME( config->m_automations.m_auto_vaccinator_projectile_prediction );

			if ( auto healing_target = ctx->m_entity_list->get_client_entity_from_handle( ctx->m_local_weapon->m_healing_target( ) )->as<c_base_player>( ) )
			{
				if ( tf2::is_visible( healing_target, entity, healing_target->get_world_space_center( ), predicted_position, MASK_SHOT | CONTENTS_GRATE )
					 || tf2::is_visible( healing_target, entity, healing_target->get_world_space_center( ), entity->get_world_space_center( ), MASK_SHOT | CONTENTS_GRATE ) )
					threat.m_threat_scale = 200;

				threat.m_distance = healing_target->m_origin( ).dist_to( predicted_position );
			}
			else
			{
				threat.m_distance = ctx->m_local_player->m_origin( ).dist_to( predicted_position );
			}

			if ( tf2::is_visible( ctx->m_local_player, entity, ctx->m_local_player->get_world_space_center( ), predicted_position, MASK_SHOT | CONTENTS_GRATE )
				 || tf2::is_visible( ctx->m_local_player, entity, ctx->m_local_player->get_world_space_center( ), entity->get_world_space_center( ), MASK_SHOT | CONTENTS_GRATE ) )
				threat.m_threat_scale = 200;

			if ( threat.m_distance > 150.f )
				continue;

			threat.m_entity = entity->as<c_base_player>( );
			threat.m_projectile = true;


			m_threats.emplace_back( threat );
			continue;
		}
	}

	std::sort(m_healing_targets.begin(), m_healing_targets.end(), [](const c_autovacc_healing& a, c_autovacc_healing& b) {

		if (a.m_priority != b.m_priority)
			return a.m_priority > b.m_priority;

		if (a.m_is_friend)
			return true;

		float a_health = static_cast<float>(a.m_entity->m_health()) / static_cast<float>(a.m_entity->get_max_health());
		float b_health = static_cast<float>(b.m_entity->m_health()) / static_cast<float>(b.m_entity->get_max_health());
		return a_health < b_health;
	});

	std::sort(m_threats.begin(), m_threats.end(), [](const c_autovacc_threat& a, c_autovacc_threat& b) {
		if (a.m_threat_scale != b.m_threat_scale)
			return a.m_threat_scale > b.m_threat_scale;

		return a.m_distance < b.m_distance;
	});
}

void c_automations::draw_autovacc_status()
{
	if (!ctx->m_local_player || !ctx->m_local_weapon || !ctx->m_local_player->is_alive())
		return;

	if (!ctx->m_engine_client->get_net_channel_info())
		return;

	if (ctx->m_local_player->m_player_class() != CLASS_MEDIC)
		return;

	if (ctx->m_local_weapon->get_slot() != 1)
		return;

	if (!config->m_automations.m_auto_vaccinator)
		return;

	int x = ctx->m_screen_width / 2;
	int y = ctx->m_screen_height / 2 + 24;

	render->outlined_text(x, y, ALIGN_CENTER, ctx->m_tahoma_bold.m_font, color(255, 255, 255), m_autovacc_status);

	int charges = floor(math::remap_val_clamped(ctx->m_local_weapon->m_charge_level(), 0.f, 1.f, 0, 4));
	render->outlined_text(x, y + ctx->m_tahoma_bold.m_tall + 5, ALIGN_CENTER, ctx->m_tahoma_bold.m_font, color(255, 255, 255), L"Charges: %d", charges);
}


void c_automations::auto_vaccinator(c_user_cmd* cmd)
{
	if (ctx->m_local_player && ctx->m_local_weapon && ctx->m_local_player->is_alive() &&
		ctx->m_engine_client->get_net_channel_info() && ctx->m_local_player->m_player_class() == CLASS_MEDIC &&
		ctx->m_local_weapon->get_slot() == 1 && config->m_automations.m_auto_vaccinator)
	{
		bool shouldPop = false;
		int requiredResistance = HITSCAN;
		static int statusUpdate = 0;

		auto healingTarget = ctx->m_entity_list->get_client_entity_from_handle(ctx->m_local_weapon->m_healing_target())->as<c_base_player>();

		gather_autovacc_data();

		if (!m_healing_targets.empty())
		{
			c_base_player* bestHealingTarget = m_healing_targets.front().m_entity;

			if (bestHealingTarget != healingTarget)
			{
				vector angleTo = math::calculate_angle(ctx->m_local_player->get_shoot_position(), bestHealingTarget->get_world_space_center());
				static int attackDelay = config->m_automations.m_auto_vac_switch_delay + 1;
				if (attackDelay-- <= 0)
				{
					cmd->m_view_angles = angleTo;
					cmd->m_buttons |= IN_ATTACK;
					attackDelay = config->m_automations.m_auto_vac_switch_delay + 1;

					ctx->m_aimbot_angles = cmd->m_view_angles;
					ctx->m_aimbot_delay = 32;
				}
				else
				{
					if (config->m_automations.m_auto_vac_switch_delay < 5)
						cmd->m_buttons &= IN_ATTACK;
				}

				if (healingTarget && healingTarget->is_player_on_friend_list())
					m_autovacc_status = "Healing";
				else
					m_autovacc_status = "Farming Charges";
			}
		}

		if (!m_threats.empty())
		{
			auto& bestThreat = m_threats.front();
			auto bestEntity = m_threats.front().m_entity;
			c_base_combat_weapon* weapon = nullptr; // Declare the weapon variable outside the switch statement

			if (bestThreat.m_projectile)
			{
				if (tf2::is_visible(ctx->m_local_player, bestEntity, ctx->m_local_player->get_world_space_center(), bestEntity->get_world_space_center(), MASK_SHOT) ||
					(healingTarget && tf2::is_visible(healingTarget, bestEntity, healingTarget->get_world_space_center(), bestEntity->get_world_space_center(), MASK_SHOT)))
				{
					requiredResistance = get_projectile_resistance_type(bestEntity);
					shouldPop = true;
				}
			}
			else
			{
				switch (bestEntity->m_player_class())
				{
				case CLASS_SNIPER:
				case CLASS_SCOUT:
				case CLASS_HEAVY:
				case CLASS_SPY:
				case CLASS_ENGINEER:
					requiredResistance = HITSCAN;
					break;
				case CLASS_PYRO:
					requiredResistance = internal_projectile_types::FIRE;
					weapon = bestEntity->get_active_weapon();
					if (weapon && weapon->get_weapon_id() == TF_WEAPON_SHOTGUN_PYRO)
						requiredResistance = internal_projectile_types::HITSCAN;
					break;
				case CLASS_SOLDIER:
					weapon = bestEntity->get_active_weapon();
					if (weapon)
					{
						if (weapon->get_slot() == 0)
							requiredResistance = internal_projectile_types::EXPLOSIVE;

						if (weapon->get_weapon_id() == TF_WEAPON_SHOTGUN_SOLDIER)
							requiredResistance = internal_projectile_types::HITSCAN;
					}
					break;
				case CLASS_DEMOMAN:
					requiredResistance = internal_projectile_types::EXPLOSIVE;
					break;
				}
			}

			if (bestThreat.m_threat_scale == 999)
			{
				requiredResistance = internal_projectile_types::HITSCAN;
				bestThreat.m_threat_scale = 1;
				shouldPop = true;
			}
		}

		if (healingTarget)
		{
			if (healingTarget->m_player_cond() & TFCond_OnFire && !healingTarget->is_fire_charged())
			{
				requiredResistance = internal_projectile_types::FIRE;
				shouldPop = true;
			}
		}

		if (ctx->m_local_player->m_player_cond() & TFCond_OnFire && !ctx->m_local_player->is_fire_charged())
		{
			requiredResistance = internal_projectile_types::FIRE;
			shouldPop = true;
		}

		if (auto_vacc_healing_data.m_should_pop && auto_vacc_healing_data.m_hurt_time > ctx->m_engine_client->time())
			shouldPop = true;

		if (auto_vacc_healing_data.m_required_resistance >= 0)
			requiredResistance = auto_vacc_healing_data.m_required_resistance;

		static int resistanceDelay = 5;
		if (ctx->m_local_weapon->m_charge_resistance_type() != requiredResistance)
		{
			m_autovacc_status= "Switching Resistance...";

			if (resistanceDelay-- <= 0)
			{
				cmd->m_buttons |= IN_RELOAD;
				resistanceDelay = 3 + TIME_TO_TICKS(ctx->m_engine_client->get_net_channel_info()->get_latency(0));
			}
		}

		if (shouldPop && ctx->m_local_weapon->m_charge_level() >= 0.25f && requiredResistance == ctx->m_local_weapon->m_charge_resistance_type() &&
			config->m_automations.m_auto_vacc_pop)
		{
			if (!healingTarget)
				healingTarget = ctx->m_local_player;

			if (ctx->m_local_weapon->m_charge_resistance_type() == internal_projectile_types::HITSCAN && !healingTarget->is_bullet_charged())
				cmd->m_buttons |= IN_ATTACK2;

			if (ctx->m_local_weapon->m_charge_resistance_type() == internal_projectile_types::EXPLOSIVE && !healingTarget->is_explosive_charged())
				cmd->m_buttons |= IN_ATTACK2;

			if (ctx->m_local_weapon->m_charge_resistance_type() == internal_projectile_types::FIRE && !healingTarget->is_fire_charged() &&
				healingTarget->m_player_class() != CLASS_PYRO)
				cmd->m_buttons |= IN_ATTACK2;

			m_autovacc_status = "Popping!";

			auto_vacc_healing_data.m_should_pop = false;
			auto_vacc_healing_data.m_required_resistance = -99;
			auto_vacc_healing_data.m_hurt_time = -1;
		}
	}
}
