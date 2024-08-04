#include "seed_prediction.h"

float c_seed_prediction::calculate_mantissa_step(float value) {
    int raw_val = reinterpret_cast<int&>(value);
    int exponent = (raw_val >> 23) & 0xFF;
    return powf(2, exponent - (127 + 23));
}

float c_seed_prediction::server_cur_time() {
    float server_time = ctx->m_globals->m_interval_per_tick * ctx->m_local_player->m_tick_base();
    return server_time;
}

bool c_seed_prediction::is_perfect_shot(c_base_combat_weapon* weapon, float provided_time) {
    auto server_time = provided_time == 0.0 ? server_cur_time() : provided_time;
    auto time_since_attack = server_time - weapon->m_last_fire_time();

    auto bullets_per_shot = weapon->get_weapon_data().m_bullets_per_shot;

    if (bullets_per_shot >= 1)
        bullets_per_shot = tf2::attrib_hook_float(bullets_per_shot, "mult_bullets_per_shot", weapon, 0x0, true);

    else bullets_per_shot = 1;

    if ((bullets_per_shot == 1 && time_since_attack > 1.25) || (bullets_per_shot > 1 && time_since_attack > 0.25))
        return true;

    return false;
}

bool c_seed_prediction::dispatch_user_message_handler(bf_read* buf, int type) {
    bool should_call_original = true;

    if ((!m_waiting_perf_data && !m_last_was_player_perf) || ctx->m_running_shift || ctx->m_warping)
        return should_call_original;

    if (type != 5)
        return should_call_original;

    int message_dest = buf->ReadByte();

    if (message_dest != 2) {
        buf->Seek(0);
        return should_call_original;
    }

    double start_time = tf2::plat_float_time();

    char msg_str[256];
    buf->ReadString(msg_str, sizeof(msg_str));
    buf->Seek(0);

    char server_time[32] = "";
    for (int i = 0; i < sizeof(server_time); i++) {
        if (msg_str[i] != ' ')
            server_time[i] = msg_str[i];
        else
            break;
    }
    char* tmp;
    double data = std::strtod(server_time, &tmp);
    m_server_time = data;
    m_mantissa_step = calculate_mantissa_step(data * 1000.0);

    if (m_mantissa_step < 1.0) {
        m_waiting_perf_data = false;
        m_last_was_player_perf = true;
        should_call_original = false;
        m_bad_mantissa = true;
        m_no_spread_synced = NOT_SYNCED;
        return should_call_original;
    }

    m_bad_mantissa = false;
    m_last_was_player_perf = true;
    should_call_original = false;

    if (!m_waiting_perf_data)
        return should_call_original;

    if (m_no_spread_synced == NOT_SYNCED) {
        double client_time = tf2::plat_float_time();
        double total_latency = (client_time - (client_time - start_time)) - m_sent_client_floattime;
        m_float_time_delta = data - m_sent_client_floattime;
        m_float_time_delta -= (total_latency / 2.0);
        m_waiting_perf_data = false;
        m_no_spread_synced = CORRECTING;
        m_is_syncing = true;
    }

    else if (m_no_spread_synced != SYNCED) {
        double time_difference = m_sent_client_floattime - data;

        double mantissa_step = calculate_mantissa_step(m_sent_client_floattime * 1000.0);
        double correction_threshhold = config->m_seed_pred.m_extreme_prediction ? 0.001 : (mantissa_step / 1000.0 / 2.0);

        if (abs(time_difference) > correction_threshhold || abs(m_last_correction) > mantissa_step / 1000.0) {
            m_float_time_delta -= time_difference;
            m_resync_needed = true;
        }

        else  m_resync_needed = false;

        m_last_correction = time_difference;
        m_waiting_perf_data = false;

        if (!m_resync_needed)
            m_no_spread_synced = SYNCED;
    }

    else {
        m_resync_needed = false;
        m_waiting_perf_data = false;
    }

    return should_call_original;
};

void c_seed_prediction::apply_spread_correction(vector& angles, int seed, float spread) {
    if (!ctx->m_local_player || !ctx->m_local_weapon || ctx->m_running_shift || ctx->m_warping)
        return;

    if (!ctx->m_local_player->is_alive())
        return;

    c_base_combat_weapon* weapon = ctx->m_local_weapon;

    bool is_first_shot_perfect = is_perfect_shot(weapon, 0.0f);

    int bullets_per_shot = weapon->get_weapon_data().m_bullets_per_shot;

    if (bullets_per_shot >= 1)
        bullets_per_shot = tf2::attrib_hook_float(bullets_per_shot, "mult_bullets_per_shot", ctx->m_local_weapon, 0x0, true);

    else bullets_per_shot = 1;

    if (bullets_per_shot == 1 && is_first_shot_perfect)
        return;

    std::vector< vector > bullet_corrections;
    vector average_spread(0.0f);

    for (int i = 0; i < bullets_per_shot; i++) {
        tf2::random_seed(seed + i);
        auto x = tf2::random_float(-0.5, 0.5) + tf2::random_float(-0.5, 0.5);
        auto y = tf2::random_float(-0.5, 0.5) + tf2::random_float(-0.5, 0.5);

        if (is_first_shot_perfect && !i) {
            x = 0.0f;
            y = 0.0f;
        }

        math::clamp_angle(angles);
        auto forward = vector(), right = vector(), up = vector();
        math::angle_vectors(angles, &forward, &right, &up);

        vector fixed_spread = forward + (right * x * spread) + (up * y * spread);
        fixed_spread.normalize_in_place();
        average_spread += fixed_spread;
        bullet_corrections.push_back(fixed_spread);
    }

    average_spread /= bullets_per_shot;
    vector fixed_spread(std::numeric_limits< float >::max());

    for (auto& spread : bullet_corrections) {
        if (spread.dist_to(average_spread) < fixed_spread.dist_to(average_spread))
            fixed_spread = spread;
    }

    vector fixed_angles;
    math::vector_angles(fixed_spread, fixed_angles);
    vector correction = (angles - fixed_angles);
    angles += correction;
    math::clamp_angle(angles);
}

bool c_seed_prediction::send_net_message_handler(i_net_message* data) {
    // if we send clc_move with playerperf command or corrected angles, we must ensure it will be sent via reliable stream
    if (m_should_update_time) {
        if (data->get_type() != clc_moveval)
            return false;

        // and wait for post call
        m_waiting_for_post_SNM = true;

        // Force reliable
        return true;
    }
    else if (m_no_spread_synced) {
        if (data->get_type() != clc_moveval)
            return false;
    }
    return false;
}

static timer wait_perf{ };

void c_seed_prediction::send_net_message_post_handler() {
    if (!m_waiting_for_post_SNM || (m_waiting_perf_data && !wait_perf.test_and_set(1000)) || ctx->m_running_shift || ctx->m_warping)
        return;

    m_waiting_for_post_SNM = false;

    ctx->m_engine_client->client_cmd_unrestricted("playerperf");

    m_should_update_time = false;

    if (m_no_spread_synced == NOT_SYNCED)
        m_sent_client_floattime = tf2::plat_float_time();

    m_ping_at_send = ctx->m_engine_client->get_net_channel_info()->get_latency(FLOW_OUTGOING);

    m_waiting_perf_data = true;
    wait_perf.update();
}


void c_seed_prediction::cl_sendmove_handler() {
    m_first_usercmd = true;
    m_called_from_sendmove = false;

    if (!m_no_spread_synced || !config->m_seed_pred.m_enabled || ctx->m_running_shift || ctx->m_warping)
        return;

    m_current_weapon_spread = 0.0;

    if (!ctx->m_local_player || !ctx->m_local_weapon || !ctx->m_local_weapon->is_spread_weapon())
        return;

    int new_packets = 1 + ctx->m_client_state->m_choked_commands;

    double asumed_real_time = tf2::plat_float_time() + m_float_time_delta;
    double predicted_time = asumed_real_time;

    predicted_time += m_write_usercmd_correction * new_packets;
    double ping = ctx->m_client_state->m_net_channel->get_latency(FLOW_OUTGOING);

    static timer s_NextCheck;

    if (s_NextCheck.check(1000) && ctx->m_local_player->is_alive() && !m_waiting_perf_data) {
        s_NextCheck.update();

        m_sent_client_floattime = asumed_real_time;
        m_should_update_time = true;

        if (!ctx->m_local_player->is_alive() && m_no_spread_synced != CORRECTING) {
            m_last_sync_delta_time = m_float_time_delta;
            m_last_ping_at_send = m_ping_at_send;
            m_no_spread_synced = DEAD_SYNC;
        }
        // We always set this, and change it later in CL_SendMove if we aren't dead
        m_resynced_this_death = true;
    }

    // If we're dead just return original
    if (!ctx->m_local_player->is_alive())
        return;
    else {
        // We are alive
        m_resynced_this_death = false;

        // We should cancel the dead sync process
        if (m_no_spread_synced == DEAD_SYNC)
        {
            // Restore delta time
            m_float_time_delta = m_last_sync_delta_time;
            m_ping_at_send = m_last_ping_at_send;
            // Mark as synced
            m_no_spread_synced = SYNCED;
            m_should_update_time = false;
        }
    }

    // Bad weapon
    if (ctx->m_local_weapon && ctx->m_local_weapon->get_slot() >= 2 || tf2::is_projectile_weapon(ctx->m_local_weapon))
        return;

    float current_time = server_cur_time();

    // Check if we are attacking, if not then no point in adjusting
    if (!ctx->was_in_attack)
        return;

    // If we have a perfect shot and we don#t want to center the whole cone, returne too
    if (ctx->m_local_weapon && is_perfect_shot(ctx->m_local_weapon, current_time))
        return;

    if (ctx->m_local_weapon) {
        m_current_weapon_spread = ctx->m_local_weapon->get_weapon_spread();
    }

    if (!std::isfinite(m_current_weapon_spread)) {
        return;
    }

    // Only adjust if not using usercmd seed
    if (!m_use_usercmd_seed)
        m_prediction_seed = (float)(predicted_time * 1000.0f);

    // Call original
    m_called_from_sendmove = true;
    //store in global for cl_sendmove_handler_post
    m_time_start = tf2::plat_float_time();
    m_new_packets = new_packets;

    //here it calls cl_sendmove original
    //rest of the code is continued in another function, which runs after calling cl_sendmove
}

void c_seed_prediction::cl_sendmove_handler_post() {

    if (!config->m_seed_pred.m_enabled || ctx->m_running_shift || ctx->m_warping)
        return;

    if (!ctx->m_local_player || !ctx->m_local_weapon || !ctx->m_local_weapon->is_spread_weapon())
        return;

    double time_end = tf2::plat_float_time();
    m_called_from_sendmove = false;

    // Update the processing time if we actually processed stuff
    if (m_should_update_usercmd_correction) {
        // How long it took for us to process each cmd? We will add this number next time we will process usercommands
        m_write_usercmd_correction = (time_end - m_time_start) / m_new_packets;
        m_should_update_usercmd_correction = false;
    }
}

static timer update_nospread_timer{};
void c_seed_prediction::create_move_handler(c_user_cmd* cmd) {
    if (ctx->m_running_shift || ctx->m_warping)
        return;

    static auto sv_usercmd_custom_random_seed = ctx->m_cvar->find_var("sv_usercmd_custom_random_seed");
    if (!sv_usercmd_custom_random_seed)
        sv_usercmd_custom_random_seed = ctx->m_cvar->find_var("sv_usercmd_custom_random_seed");

    else if (!sv_usercmd_custom_random_seed->get_bool())
    {
        auto seed = md5_pseudo_random(cmd->m_command_number) & 0x7FFFFFFF;
        m_prediction_seed = *reinterpret_cast<float*>(&seed);
        m_use_usercmd_seed = true;
    }
    // Normal server
    else
        m_use_usercmd_seed = false;

    if (m_no_spread_synced == SYNCED)
        m_is_syncing = false;

    // Not synced currently, try to sync //stops here 1
    if (m_no_spread_synced == NOT_SYNCED && !m_bad_mantissa)
    {
        m_is_syncing = true;
        m_should_update_time = true;
        update_nospread_timer.update();
        //i::cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "[CreateMove] Not synced currently, try to sync\n");
    }
    // Else if mantissa bad, update every 10 mins
    else if (m_no_spread_synced == NOT_SYNCED && update_nospread_timer.test_and_set(1000))
        m_no_spread_synced = CORRECTING;

    if ((m_no_spread_synced != SYNCED && !m_resync_needed) || m_current_weapon_spread == 0.0 || !config->m_seed_pred.m_enabled)
        return;

    if (!(cmd->m_buttons & IN_ATTACK)) {
        m_user_cmd_backup = *cmd;
        m_first_usercmd = false;
        return;
    }

    apply_spread_correction(cmd->m_view_angles, reinterpret_cast<int&>(m_prediction_seed) & 0xFF, m_current_weapon_spread);
    m_should_update_usercmd_correction = true;

    m_user_cmd_backup = *cmd;
    m_first_usercmd = false;
}

void c_seed_prediction::reset_variables() {
    m_waiting_perf_data = false;
    m_should_update_time = false;
    m_waiting_for_post_SNM = false;
    m_resync_needed = false;
    m_is_syncing = false;
    m_last_was_player_perf = false;
    m_resynced_this_death = false;
    m_no_spread_synced = NOT_SYNCED; // this will be set to 0 each level init / level shutdown
    m_bad_mantissa = false;      // Also reset every levelinit/shutdown
    m_float_time_delta = 0.0;
    m_ping_at_send = 0.0;
    m_last_ping_at_send = 0.0;
    m_sent_client_floattime = 0.0;
    m_last_correction = 0.0;
    m_write_usercmd_correction = 0.0;
    m_last_sync_delta_time = 0.0;
    m_prediction_seed = 0.0;
    m_time_start = 0.0;
    m_use_usercmd_seed = false;
    m_current_weapon_spread = 0.0;
    m_first_usercmd = false;
    m_called_from_sendmove = false;
    m_should_update_usercmd_correction = false;
    m_mantissa_step = 0.0;
    m_server_time = 0.0f;
    m_new_packets = 0;
    m_user_cmd_backup = { };
}