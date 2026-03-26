#pragma once

#include "queue.hpp"
#include "session.hpp"
#include "shot_result.hpp"
#include "utils/thread_pool.hpp"
#include "database/database_manager.hpp"
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <functional>

class SessionManager {
public:
    SessionManager(ThreadPool& pool, DatabaseManager& db);

    ~SessionManager();

    void add_to_queue_async(std::shared_ptr<Player> player);

    void make_move_async(int player_id, int game_id, int x, int y, std::function<void(ShotResult)> callback);

    void place_ship_async(int player_id, int game_id, int length, int x, int y, bool is_horizontal, std::function<void(bool)> callback);

    void clear_field_async(int player_id, int game_id, std::function<void(bool)> callback);

    void set_player_ready(int player_id, int game_id);

    std::shared_ptr<Session> get_session(int game_id);
    
    int get_player_game(int player_id);
    
    void player_disconnected_async(int player_id);
    
    void check_timeouts();

private:
    void try_match_players();
    
    void create_game_session(std::shared_ptr<Player> p1, std::shared_ptr<Player> p2);
    
    void end_game(int game_id);

    Queue queue_;
    std::unordered_map<int, std::shared_ptr<Session>> sessions_;
    std::unordered_map<int, int> player_to_game_;
    ThreadPool& thread_pool_;
    DatabaseManager& db_manager_;
    std::atomic<int> next_game_id_{0};

    std::mutex sessions_mutex_;
};
