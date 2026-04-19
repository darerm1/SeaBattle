#pragma once

#include <memory>
#include <chrono>
#include <mutex>
#include "player.hpp"
#include "shot_result.hpp"
#include "game_status.hpp"
#include "game_field.hpp"

class Session {
public:
    Session(int game_id, std::shared_ptr<Player> p1, std::shared_ptr<Player> p2);

    ~Session();

    bool place_ship(int player_id, int length, int x, int y, bool is_horizontal);

    bool clear_field(int player_id);

    ShotResult make_move(int player_id, int x, int y);

    bool check_timeout();

    bool set_player_ready(int player_id);

    std::pair<int, int> calculate_ratings() const;

    void apply_new_ratings(int first_rating_new, int second_rating_new);

    void end_game();

    std::pair<int, int> get_player_ids() const;

    int get_game_id() const;

    bool game_is_over() const;
    
    int get_winner_id() const;
    
    int get_current_turn() const;

    const GameField& get_first_field() const;

    const GameField& get_second_field() const;

    const std::string& get_first_login() const;

    const std::string& get_second_login() const;

    bool game_is_started() const;

private:
    GameField& get_player_field(int player_id);
    
    GameField& get_opponent_field(int player_id);
    
    int game_id_;
    GameStatus game_status_;
    GameField first_field_;
    GameField second_field_;
    std::shared_ptr<Player> first_player_;
    std::shared_ptr<Player> second_player_;
    int turn_ = -1;
    int winner_id_ = -1;
    bool first_ready_ = false;
    bool second_ready_ = false;
    std::chrono::steady_clock::time_point turn_start_time_;
    const std::chrono::seconds turn_timeout_{5};
    std::mutex session_mutex_;
};
