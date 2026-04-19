#include "database/database_manager.hpp"
#include "core/player.hpp"
#include "logger/logger.hpp"

DatabaseManager::DatabaseManager(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        Logger::log("Database failed to open");
        db_ = nullptr;
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "login TEXT UNIQUE,"
                        "password_hash TEXT,"
                        "rating INTEGER DEFAULT 0)";
    
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        Logger::log("Failed to create users table: ", errMsg ? errMsg : "unknown error");
        sqlite3_free(errMsg);
    }
}

DatabaseManager::~DatabaseManager() {
    sqlite3_close(db_);
}

std::shared_ptr<Player> DatabaseManager::authenticate(const std::string& login, const std::string& password) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!db_) {
        Logger::log("DB did not open in authenticate");
        return nullptr;
    }

    const char* sql = "SELECT id, password_hash, rating FROM users WHERE login = ?";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::log("Failed to authenticate user. DB error: ", sqlite3_errmsg(db_));
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_TRANSIENT);

    std::shared_ptr<Player> player = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* db_pass = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int rating = sqlite3_column_int(stmt, 2);

        if (password == db_pass) {
            player = std::make_shared<Player>(id, login, "");
            player->set_points(rating);
        }
    }
    
    sqlite3_finalize(stmt);
    return player;
}

bool DatabaseManager::updateRatings(int first_player_id, int first_new_rating, int second_player_id, int second_new_rating) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    if (!db_) {
        Logger::log("DB did not open in updateRating");
        return false;
    }

    char* errMsg = nullptr;
    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);

    auto updateFunc = [&](int id, int rating) {
        const char* sql = "UPDATE users SET rating = ? WHERE id = ?";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
        
        sqlite3_bind_int(stmt, 1, rating);
        sqlite3_bind_int(stmt, 2, id);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE) && (sqlite3_changes(db_) > 0);
        sqlite3_finalize(stmt);
        return success;
    };

    if (updateFunc(first_player_id, first_new_rating) && updateFunc(second_player_id, second_new_rating)) {
        sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &errMsg);
        return true;
    } else {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, &errMsg);
        Logger::log("Database transaction failed and rolled back");
        return false;
    }
}

std::shared_ptr<Player> DatabaseManager::createUser(const std::string& login, const std::string& password_hash) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    if (!db_) {
        Logger::log("DB not open in createUser");
        return nullptr;
    }

    if (login.empty() || password_hash.empty()) {
        Logger::log("createUser failed: empty login or password");
        return nullptr;
    }
    
    const char* sql = "INSERT INTO users (login, password_hash, rating) VALUES (?, ?, 1000)";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::log("Failed to prepare statement in createUser: ", sqlite3_errmsg(db_));
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_TRANSIENT);

    std::shared_ptr<Player> result = nullptr;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        int id = static_cast<int>(sqlite3_last_insert_rowid(db_));
        result = std::make_shared<Player>(id, login, "");
        result->set_points(1000);
    } else {
        Logger::log("Failed to create user ", login, ": ", sqlite3_errmsg(db_));
    }

    sqlite3_finalize(stmt);
    return result;
}
