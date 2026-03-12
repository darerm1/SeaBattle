#include "core/session_manager.hpp"
#include "logger/logger.hpp"
#include <algorithm>

SessionManager::SessionManager() = default;

SessionManager::~SessionManager() = default;

void SessionManager::add_to_queue(std::shared_ptr<Player> player) {
    queue_.push_back(player);
    Logger::log("Player ", player->get_id(), " added in queue");
    try_match_players();
}

ShotResult SessionManager::make_move(int player_id, int game_id, int x, int y) {
    std::shared_ptr<Session> session = get_session(game_id);
    if (!session) {
        Logger::log("ERROR: Game ", game_id, " not found for player ", player_id);
        return ShotResult::INVALID;
    }
    
    ShotResult result = session->make_move(player_id, x, y);
    
    if (session->game_is_over()) {
        end_game(game_id);
    }
    
    return result;
}

bool SessionManager::place_ship(int player_id, int game_id, int length, int x, int y, bool is_horizontal) {
    std::shared_ptr<Session> session = get_session(game_id);
    if (!session) {
        Logger::log("ERROR: Game ", game_id, " not found for ship placement");
        return false;
    }

    bool result = session->place_ship(player_id, length, x, y, is_horizontal);
    Logger::log("Ship placement ", result ? "successful" : "failed");    
    return result;
}

bool SessionManager::clear_field(int player_id, int game_id) {
    Logger::log("Player ", player_id, " clearing field in game ", game_id);
    std::shared_ptr<Session> session = get_session(game_id);
    if (!session) {
        Logger::log("ERROR: Game ", game_id, " not found for field clearing");
        return false;
    }
    
    session->clear_field(player_id);
    Logger::log("Field cleared for player ", player_id);
    return true;
}

void SessionManager::set_player_ready(int player_id, int game_id) {
    std::shared_ptr<Session> session = get_session(game_id);
    if (!session) {
        Logger::log("ERROR: Game ", game_id, " not found for player ready");
        return;
    }
    session->set_player_ready(player_id);
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

void SessionManager::player_disconnected(int player_id) {
    queue_.remove(player_id);
    Logger::log("Player ", player_id, " removed from queue");
    
    int game_id = get_player_game(player_id);
    if (game_id != -1) {
        Logger::log("Player ", player_id, " interrupted game ", game_id);
        end_game(game_id);
    }
    Logger::log("Player ", player_id, " disconnected");
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
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    int game_id;
    if (sessions_.empty()) game_id = 0;
    else game_id = sessions_[sessions_.size()-1]->get_game_id() + 1;
    
    Logger::log("Creating game session ", game_id, " for players ", p1->get_id(), " and ", p2->get_id());
    std::shared_ptr<Session> session = std::make_shared<Session>(game_id, p1, p2);
    
    sessions_[game_id] = session;
    player_to_game_[p1->get_id()] = game_id;
    player_to_game_[p2->get_id()] = game_id;
}

void SessionManager::end_game(int game_id) {
    Logger::log("Ending game ", game_id);
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    auto it = sessions_.find(game_id);
    if (it == sessions_.end()) { 
        Logger::log("ERROR: Game ", game_id, " not found for ending");
        return;
    }
    
    std::shared_ptr<Session> session = it->second;

    session->end_game();
    
    auto [first_player_id, second_player_id] = session->get_player_ids();
    
    player_to_game_.erase(first_player_id);
    player_to_game_.erase(second_player_id);

    sessions_.erase(it);
    Logger::log("Game ", game_id, " ended. Winner: ", session->get_winner_id(), ". Players: ", first_player_id, ", ", second_player_id);
}
