#pragma once
#include "../../../include/tf2.h"
#include <deque>

class c_movement_sim {
public:
    std::map< c_base_player*, std::deque< vector > > m_directions = { };
    std::map< c_base_player*, bool > m_ground_state = { };
    bool init(c_base_player* player);
    void restore();
    bool validate_player(c_base_player* player) const;
    void clear_state();
    void setup_player(c_base_player* player);
    void adjust_player_flags(c_base_player* player);
    bool should_adjust_model_scale(c_base_player* player) const;
    void adjust_model_scale(c_base_player* player);
    void adjust_player_origin(c_base_player* player);
    bool should_adjust_velocity(c_base_player* player) const;
    void adjust_velocity(c_base_player* player);
    bool should_restore_due_to_hitchance(c_base_player* player) const;
    void reset_player_command() const;
    void restore_prediction_state() const;
    vector run_tick();
    void update_prediction();
    void apply_turn_prediction();
    void apply_turn_speed_clamp(c_base_player* player);
    void fill_directions();
    void draw_path();

private:
    std::vector< vector > m_positions = { };
    c_base_player* m_player = nullptr;
    c_move_data m_move_data = { };
    bool m_old_in_prediction = false, m_old_first_time_predicted = false;
    float m_old_frame_time = 0.0f, m_turn_speed = 0.0f, m_old_forward_move = 0.0f, m_old_side_move = 0.0f, m_time_stamp = 0.0f;
    inline const vector& get_origin() { return m_move_data.m_abs_origin; }
    void setup_move_data(c_base_player* player, c_move_data* move_data);
    void initialize_move_data(c_base_player* player, c_move_data* move_data);
    void update_old_move_values(c_move_data* move_data);
    float calculate_forward_move(c_move_data* move_data, const vector& right, const vector& forward);
    float calculate_side_move(c_move_data* move_data, const vector& forward, const vector& right);
    void compute_turn_speed(c_base_player* player, c_move_data* move_data);
    float calculate_average_turn_speed(c_base_player* player, c_move_data* move_data);
    bool is_valid_player(c_base_player* player);
    void update_player_directions(c_base_player* player);
    bool is_player_on_ground(c_base_player* player);
    void clear_player_directions(c_base_player* player);
    void add_player_velocity_to_directions(c_base_player* player);
    void update_ground_state(c_base_player* player);
    void remove_old_directions(c_base_player* player);
    void update_time_stamp_and_positions();
};

inline c_movement_sim* movement_sim = new c_movement_sim;
