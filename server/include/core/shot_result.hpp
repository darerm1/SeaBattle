#pragma once

enum class ShotResult {
    HIT,
    MISS,
    SUNK,
    GAME_OVER,
    RESHOT,
    INVALID
};

inline const char* shot_result_to_string(ShotResult result) {
    switch(result) {
        case ShotResult::HIT: return "HIT";
        case ShotResult::MISS: return "MISS";
        case ShotResult::SUNK: return "SUNK";
        case ShotResult::GAME_OVER: return "GAME_OVER";
        case ShotResult::RESHOT: return "RESHOT";
        case ShotResult::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}
