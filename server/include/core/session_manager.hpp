#pragma once

#include "queue.hpp"
#include "session.hpp"
#include <mutex>
#include <map>

class SessionManager {
public:
    SessionManager();

    ~SessionManager();

    void add_to_queue(std::shared_ptr<Player> player);

    ShotResult make_move(int player_id, int game_id, int x, int y);

    bool place_ship(int player_id, int game_id, int length, int x, int y, bool is_horizontal);

    bool clear_field(int player_id, int game_id);

    void set_player_ready(int player_id, int game_id);

    std::shared_ptr<Session> get_session(int game_id);
    
    int get_player_game(int player_id);
    
    void player_disconnected(int player_id);
    
    void check_timeouts();

private:
    void try_match_players();
    
    void create_game_session(std::shared_ptr<Player> p1, std::shared_ptr<Player> p2);
    
    void end_game(int game_id);

    Queue queue_;
    std::vector<std::shared_ptr<Session>> sessions_;
    std::map<int, int> player_to_game_;

    std::mutex queue_mutex_;
    std::mutex sessions_mutex_;
};
