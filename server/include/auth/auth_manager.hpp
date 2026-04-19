#pragma once

#include "core/player.hpp"
#include "database/database_manager.hpp"
#include "utils/thread_pool.hpp"
#include <memory>
#include <functional>
#include <string>

class AuthManager {
public:
    AuthManager(DatabaseManager& db, ThreadPool& pool);

    ~AuthManager();
    
    void authenticate_async(const std::string& login, const std::string& password, std::function<void(std::shared_ptr<Player>)> callback);

    void register_user_async(const std::string& login, const std::string& password, std::function<void(std::shared_ptr<Player>)> callback);
    
private:
    DatabaseManager& database_manager_;
    ThreadPool& thread_pool_;
};
