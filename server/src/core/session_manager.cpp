#include "core/session_manager.hpp"
#include "logger/logger.hpp"
#include <algorithm>

SessionManager::SessionManager() = default;

SessionManager::~SessionManager() = default;

void SessionManager::add_to_queue(std::shared_ptr<Player> player) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push_back(player);
    player->set_status(PlayerStatus::WAITING);
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
    
    const auto& first_field = session->get_first_field();
    const auto& second_field = session->get_second_field();
    const std::string& first_login = session->get_first_login();
    const std::string& second_login = session->get_second_login();
    
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
    const auto& first_field = session->get_first_field();
    const auto& second_field = session->get_second_field();
    const std::string& first_login = session->get_first_login();
    const std::string& second_login = session->get_second_login();
    
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
    const auto& first_field = session->get_first_field();
    const auto& second_field = session->get_second_field();
    const std::string& first_login = session->get_first_login();
    const std::string& second_login = session->get_second_login();
    return true;
}

void SessionManager::set_player_ready(int player_id, int game_id) {
    std::shared_ptr<Session> session = get_session(game_id);
    if (!session) {
        Logger::log("ERROR: Game ", game_id, " not found for player ready");
        return;
    }
    session->set_player_ready(player_id);
    if (session->game_is_started()) {
        const auto& first_field = session->get_first_field();
        const auto& second_field = session->get_second_field();
        const auto& first_login = session->get_first_login();
        const auto& second_login = session->get_second_login();
    }
}

std::shared_ptr<Session> SessionManager::get_session(int game_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = std::find_if(sessions_.begin(), sessions_.end(), [game_id](const auto& session) { return session->get_game_id() == game_id; });
    
    if (it != sessions_.end()) {
        return *it;
    }
    return nullptr;
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
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_.remove(player_id);
        Logger::log("Player ", player_id, " removed from queue");
    }
    
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
        for (auto& session_ptr : sessions_) {
            if (session_ptr->check_timeout()) {
                int game_id = session_ptr->get_game_id();
                games_to_end.push_back(game_id);
                Logger::log("Game ", game_id, " timeout detected");
            }
        }
    }
    
    for (int game_id : games_to_end) {
        end_game(game_id);
    }
}

void SessionManager::try_match_players() { // очередь лочится в add_to_queue, откуда вызывается этот метод. При использовании его где-то ещё, помнить о локе очереди и что тут его нет
    //std::lock_guard<std::mutex> lock(queue_mutex_);
    Logger::log("Trying to match players. Queue size: ", queue_.size());
    while (queue_.size() >= 2) {
        std::shared_ptr<Player> first_player = queue_.pop_front();
        std::shared_ptr<Player> second_player = queue_.find_opponent(first_player);
        if (!second_player) {
            Logger::log("No suitable opponent found for player ", first_player->get_id(), " - returning to queue");
            queue_.push_back(first_player);
            break;
        }
        
        Logger::log("Matched players ", first_player->get_id(), " and ", second_player->get_id());
        create_game_session(first_player, second_player);
    }
}

void SessionManager::create_game_session(std::shared_ptr<Player> p1, std::shared_ptr<Player> p2) {
    int game_id;
    if (sessions_.empty()) game_id = 0;
    else game_id = sessions_[sessions_.size()-1]->get_game_id() + 1;
    
    Logger::log("Creating game session ", game_id, " for players ", p1->get_id(), " and ", p2->get_id());
    std::shared_ptr<Session> session = std::make_shared<Session>(game_id, p1, p2);
    
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.push_back(session);
        player_to_game_[p1->get_id()] = game_id;
        player_to_game_[p2->get_id()] = game_id;
    }
}

void SessionManager::end_game(int game_id) {
    Logger::log("Ending game ", game_id);
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    auto it = std::find_if(sessions_.begin(), sessions_.end(), [game_id](const auto& session_ptr) { return session_ptr->get_game_id() == game_id; });
    if (it == sessions_.end()) { 
        Logger::log("ERROR: Game ", game_id, " not found for ending");
        return;
    }
    
    std::shared_ptr<Session> session = *it;

    session->end_game();
    
    auto [first_player_id, second_player_id] = session->get_player_ids();
    
    player_to_game_.erase(first_player_id);
    player_to_game_.erase(second_player_id);

    sessions_.erase(it);
    Logger::log("Game ", game_id, " ended. Winner: ", session->get_winner_id(), ". Players: ", first_player_id, ", ", second_player_id);
}
