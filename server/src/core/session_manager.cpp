#include "core/session_manager.hpp"
#include "logger/logger.hpp"
#include <algorithm>

SessionManager::SessionManager(ThreadPool& pool, DatabaseManager& db) : thread_pool_(pool), db_manager_(db) {}

SessionManager::~SessionManager() = default;

void SessionManager::set_notify_callback(std::function<void(int, const std::string&)> fn) {
    Logger::log("SessionManager::set_notify_callback called");
    notify_player_ = std::move(fn);
}

void SessionManager::add_to_queue_async(std::shared_ptr<Player> player) {
    thread_pool_.enqueue([this, player]() {
        queue_.push_back(player);
        Logger::log("Player ", player->get_id(), " added in queue");
        try_match_players();
    });
}

void SessionManager::make_move_async(int player_id, int game_id, int x, int y, std::function<void(ShotResult)> callback) {
    thread_pool_.enqueue([this, player_id, game_id, x, y]() {
        std::shared_ptr<Session> session = get_session(game_id);
        if (!session) {
            Logger::log("ERROR: Game ", game_id, " not found for player ", player_id);
            return ShotResult::INVALID;
        }

        ShotResult result = session->make_move(player_id, x, y);

        auto [p1, p2] = session->get_player_ids();
        int opponent_id = (player_id == p1) ? p2 : p1;

        if (notify_player_ && result != ShotResult::INVALID) {
            notify_player_(opponent_id, "Opponent move: " + std::to_string(x) + " " +
                std::to_string(y) + " " + std::to_string(static_cast<int>(result)) + "\n");
        }

        if (session->game_is_over()) {
            int winner = session->get_winner_id();
            if (notify_player_) {
                notify_player_(p1, "Game over: " + std::to_string(winner) + "\n");
                notify_player_(p2, "Game over: " + std::to_string(winner) + "\n");
            }
            end_game(game_id);
        }

        return result;
    }, callback);
}

void SessionManager::place_ship_async(int player_id, int game_id, int length, int x, int y, bool is_horizontal, std::function<void(bool)> callback) {
    thread_pool_.enqueue([this, player_id, game_id, length, x, y, is_horizontal]() {
        std::shared_ptr<Session> session = get_session(game_id);
        if (!session) {
            Logger::log("ERROR: Game ", game_id, " not found for ship placement");
            return false;
        }

        bool result = session->place_ship(player_id, length, x, y, is_horizontal);
        Logger::log("Ship placement ", result ? "successful" : "failed");    
        return result;
    }, callback);
}

void SessionManager::clear_field_async(int player_id, int game_id, std::function<void(bool)> callback) {
    thread_pool_.enqueue([this, player_id, game_id]() {
        Logger::log("Player ", player_id, " clearing field in game ", game_id);
        std::shared_ptr<Session> session = get_session(game_id);
        if (!session) {
            Logger::log("ERROR: Game ", game_id, " not found for field clearing");
            return false;
        }
        
        session->clear_field(player_id);
        Logger::log("Field cleared for player ", player_id);
        return true;
    }, callback);
}

void SessionManager::set_player_ready(int player_id, int game_id) {
    return thread_pool_.enqueue([this, player_id, game_id]() {
        std::shared_ptr<Session> session = get_session(game_id);
        if (!session) {
            Logger::log("ERROR: Game ", game_id, " not found for player ready");
            return;
        }
        bool both_ready = session->set_player_ready(player_id);
        if (both_ready && notify_player_) {
            auto [p1, p2] = session->get_player_ids();
            int first_turn = session->get_turn_player_id();
            std::string l1 = session->get_first_login();
            std::string l2 = session->get_second_login();
            std::string r1 = std::to_string(session->get_first_rating());
            std::string r2 = std::to_string(session->get_second_rating());
            notify_player_(p1, "Game started\n");
            notify_player_(p2, "Game started\n");
            notify_player_(p1, "Game info: " + l1 + " " + r1 + " " + l2 + " " + r2 + "\n");
            notify_player_(p2, "Game info: " + l2 + " " + r2 + " " + l1 + " " + r1 + "\n");
            notify_player_(first_turn, "Your turn\n");
        }
    });
}

void SessionManager::forfeit_game_async(int player_id, int game_id, std::function<void()> callback) {
    thread_pool_.enqueue([this, player_id, game_id, callback]() {
        std::shared_ptr<Session> session = get_session(game_id);
        if (!session || session->game_is_over()) {
            if (callback) callback();
            return;
        }

        session->forfeit(player_id);
        int winner_id = session->get_winner_id();
        auto [p1, p2] = session->get_player_ids();
        int opponent_id = (player_id == p1) ? p2 : p1;
        if (notify_player_) {
            notify_player_(opponent_id, "Game over: " + std::to_string(winner_id) + "\n");
        }
        end_game(game_id);
        if (callback) callback();
    });
}

std::shared_ptr<Session> SessionManager::get_session(int game_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(game_id);
    return it != sessions_.end() ? it->second : nullptr;
}

int SessionManager::get_player_game(int player_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = player_to_game_.find(player_id);
    if (it != player_to_game_.end()) {
        return it->second;
    }
    return -1;
}

void SessionManager::player_disconnected_async(int player_id) {
    thread_pool_.enqueue([this, player_id]() {
        queue_.remove(player_id);
        Logger::log("Player ", player_id, " removed from queue");

        int game_id = get_player_game(player_id);
        if (game_id != -1) {
            Logger::log("Player ", player_id, " interrupted game ", game_id);
            std::shared_ptr<Session> session = get_session(game_id);
            if (session && !session->game_is_over()) {
                auto [p1, p2] = session->get_player_ids();
                int opponent_id = (player_id == p1) ? p2 : p1;

                if (session->game_is_setup()) {
                    if (notify_player_) {
                        notify_player_(opponent_id, "Opponent disconnected\n");
                    }
                    cancel_game(game_id);
                } else {
                    session->forfeit(player_id);
                    int winner_id = session->get_winner_id();
                    if (notify_player_) {
                        notify_player_(opponent_id, "Game over: " + std::to_string(winner_id) + "\n");
                    }
                    end_game(game_id);
                }
            }
        }
        Logger::log("Player ", player_id, " disconnected");
    });
}

void SessionManager::cancel_game(int game_id) {
    Logger::log("Cancelling game ", game_id, " (no rating change)");
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(game_id);
    if (it == sessions_.end()) return;
    auto [p1, p2] = it->second->get_player_ids();
    player_to_game_.erase(p1);
    player_to_game_.erase(p2);
    sessions_.erase(it);
}

void SessionManager::check_timeouts() {
    std::vector<int> games_to_end;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);        
        for (auto& [id, session] : sessions_) {
            if (session->check_timeout()) {
                int game_id = session->get_game_id();
                games_to_end.push_back(game_id);
                Logger::log("Game ", game_id, " timeout detected");
            }
        }
    }
    
    for (int game_id : games_to_end) {
        end_game(game_id);
    }
}

void SessionManager::try_match_players() {
    Logger::log("Trying to match players. Queue size: ", queue_.size());
    while (queue_.size() >= 2) {
        auto players = queue_.make_game_pair();
        if (players.first == nullptr || players.second == nullptr) {
            Logger::log("Failed to match players. Queue size: ", queue_.size());
            break;
        }
        
        Logger::log("Matched players ", players.first->get_id(), " and ", players.second->get_id());
        create_game_session(players.first, players.second);
    }
}

void SessionManager::create_game_session(std::shared_ptr<Player> p1, std::shared_ptr<Player> p2) {
    int game_id = next_game_id_.fetch_add(1);

    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        Logger::log("Creating game session ", game_id, " for players ", p1->get_id(), " and ", p2->get_id());
        std::shared_ptr<Session> session = std::make_shared<Session>(game_id, p1, p2);
        sessions_[game_id] = session;
        player_to_game_[p1->get_id()] = game_id;
        player_to_game_[p2->get_id()] = game_id;
    }

    Logger::log("notify_player_ is ", (notify_player_ ? "set" : "null"));

    if (notify_player_) {
        Logger::log("notify_player_ is set, sending...");
        Logger::log("Sending Opponent found to player ", p1->get_id());
        notify_player_(p1->get_id(), "Opponent found\n");
        Logger::log("Sending Opponent found to player ", p2->get_id());
        notify_player_(p2->get_id(), "Opponent found\n");
    } else {
        Logger::log("notify_player_ is NULL!");
        Logger::log("ERROR: notify_player_ is not set!");
    }
}

void SessionManager::end_game(int game_id) {
    Logger::log("Ending game ", game_id);
    std::shared_ptr<Session> session = nullptr;
    int first_player_id, second_player_id, first_rating_new, second_rating_new;

    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        
        auto it = sessions_.find(game_id);
        if (it == sessions_.end()) { 
            Logger::log("ERROR: Game ", game_id, " not found for ending");
            return;
        }
        
        session = it->second;
        
        std::tie(first_rating_new, second_rating_new) = session->calculate_ratings();
        std::tie(first_player_id, second_player_id) = session->get_player_ids();

        player_to_game_.erase(first_player_id);
        player_to_game_.erase(second_player_id);
        sessions_.erase(it); 
    }
    
    thread_pool_.enqueue([this, first_player_id, second_player_id, first_rating_new, second_rating_new, session, game_id]() {
        session->end_game();

        bool result = db_manager_.updateRatings(first_player_id, first_rating_new, second_player_id, second_rating_new);
        if (result) {
            Logger::log("Ratings saved to DB successfully.");
        } else {
            Logger::log("DB failed to update ratings.");
        }

        if (notify_player_) {
            notify_player_(first_player_id, "Rating: " + std::to_string(first_rating_new) + "\n");
            notify_player_(second_player_id, "Rating: " + std::to_string(second_rating_new) + "\n");
        }

        Logger::log("Game ", game_id, " ended. Winner: ", session->get_winner_id(), ". Players: ", first_player_id, ", ", second_player_id);
    });
}
