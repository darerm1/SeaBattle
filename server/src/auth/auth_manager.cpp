#include "auth/auth_manager.hpp"

AuthManager::AuthManager(DatabaseManager& db, ThreadPool& pool)
                        : database_manager_(db), thread_pool_(pool) {}

AuthManager::~AuthManager() = default;

void AuthManager::authenticate_async(const std::string& login, const std::string& password, std::function<void(std::shared_ptr<Player>)> callback) {
    thread_pool_.enqueue([this, login, password]() {
        return database_manager_.authenticate(login, password);
    }, 
    callback);
}

void AuthManager::register_user_async(const std::string& login, const std::string& password, std::function<void(std::shared_ptr<Player>)> callback) {
    thread_pool_.enqueue([this, login, password]() {
        return database_manager_.createUser(login, password);
    }, callback);
}
