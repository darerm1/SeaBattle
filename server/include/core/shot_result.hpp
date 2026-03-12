#pragma once

enum class ShotResult {
    HIT,
    MISS,
    INVALID,
    GAME_OVER,
    RESHOT
};

inline const char* shot_result_to_string(ShotResult result) {
    switch(result) {
        case ShotResult::HIT: return "HIT";
        case ShotResult::MISS: return "MISS";
        case ShotResult::INVALID: return "INVALID";
        case ShotResult::GAME_OVER: return "GAME_OVER";
        case ShotResult::RESHOT: return "RESHOT";
        default: return "UNKNOWN";
    }
}
