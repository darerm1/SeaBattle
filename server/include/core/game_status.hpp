#pragma once

enum class GameStatus {
    IN_PROGRESS,
    FINISHED,
    SETUP
};

inline const char* game_status_to_string(GameStatus status) {
    switch(status) {
        case GameStatus::IN_PROGRESS: return "IN_PROGRESS";
        case GameStatus::FINISHED: return "FINISHED";
        case GameStatus::SETUP: return "SETUP";
        default: return "UNKNOWN";
    }
}
