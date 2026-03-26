#pragma once

#include "core/player.hpp"
#include <sqlite3.h>
#include <memory>
#include <functional>
#include <mutex>

class DatabaseManager {
public:
    DatabaseManager(const std::string& db_path);
    
    ~DatabaseManager();

    std::shared_ptr<Player> authenticate(const std::string& login, const std::string& password);
    
    bool updateRatings(int first_player_id, int first_new_rating, int second_player_id, int second_new_rating);

    std::shared_ptr<Player> createUser(const std::string& login, const std::string& password_hash);
    
private:
    sqlite3* db_;
    std::mutex db_mutex_;
};
