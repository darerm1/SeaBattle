#pragma once

#include "core/player.hpp"
#include "database/database_manager.hpp"
#include "utils/thread_pool.hpp"
#include <boost/thread/future.hpp>
#include <memory>
#include <string>

class AuthManager {
public:
    AuthManager(DatabaseManager& db, ThreadPool& pool);

    ~AuthManager();
    
    boost::future<std::shared_ptr<Player>> authenticate(const std::string& login, const std::string& password);
    
private:
    DatabaseManager& database_manager_;
    ThreadPool& thread_pool_;
};
