#include "auth/auth_manager.hpp"

AuthManager::AuthManager(DatabaseManager& db, ThreadPool& pool)
                        : database_manager_(db), thread_pool_(pool) {}

AuthManager::~AuthManager() = default;

boost::future<std::shared_ptr<Player>> AuthManager::authenticate(const std::string& login, const std::string& password) {
    return thread_pool_.enqueue([this, login, password]() {
        return database_manager_.authenticate(login, password);
    });
}
