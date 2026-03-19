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

bool DatabaseManager::updateRating(int player_id, int new_rating) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    if (!db_) {
        Logger::log("DB did not open in updateRating");
        return false;
    }

    const char* sql = "UPDATE users SET rating = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::log("Failed to prepare statement in updateRating: ", sqlite3_errmsg(db_));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, new_rating);
    sqlite3_bind_int(stmt, 2, player_id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
    if (!success) {
        Logger::log("Error updating rating for player ", player_id, ": ", sqlite3_errmsg(db_));
    }

    sqlite3_finalize(stmt);
    return success;
}

std::shared_ptr<Player> DatabaseManager::createUser(const std::string& login, const std::string& password_hash) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    if (!db_) {
        Logger::log("DB not open in createUser");
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
