#include "core/session.hpp"
#include "logger/logger.hpp"
#include <cmath>
#include <algorithm>

Session::Session(int game_id, std::shared_ptr<Player> p1, std::shared_ptr<Player> p2) 
    : game_id_(game_id), first_player_(p1), second_player_(p2) {
    
    if (p1->get_points() <= p2->get_points()) {
        turn_ = p1->get_id();
    } else {
        turn_ = p2->get_id();
    }
    
    p1->set_status(PlayerStatus::SETUP);
    p2->set_status(PlayerStatus::SETUP);
    game_status_ = GameStatus::SETUP;

    Logger::log("Session ", game_id_, " created: players ", p1->get_id(), " (", p1->get_points(), 
            " pts) and ", p2->get_id(), " (", p2->get_points(), " pts). First turn: ", turn_);
}

Session::~Session() = default;

bool Session::place_ship(int player_id, int length, int x, int y, bool is_horizontal) {
    std::lock_guard<std::mutex> lock(session_mutex_);
    if (game_status_ != GameStatus::SETUP) {
            Logger::log("Session ", game_id_, ": ERROR - player ", player_id, 
                " tried to place ship but game is not in SETUP state");
        return false;
    }
    GameField& field = get_player_field(player_id);
    Logger::log("Player ", player_id, " placing ship: length=", length, 
        " at (", x, ",", y, ") horizontal=", is_horizontal);
    if (field.set_ship(length, x, y, is_horizontal)) {
        return true;
    }
    return false;
}

bool Session::clear_field(int player_id) {
    std::lock_guard<std::mutex> lock(session_mutex_);
    if (game_status_ != GameStatus::SETUP) {
        Logger::log("Session ", game_id_, ": ERROR - cannot clear field, game is not in SETUP");
        return false;
    }

    if (player_id == first_player_->get_id()) {
        first_field_.clear_field();
        return true;
    }
    second_field_.clear_field();
    return true;
}

ShotResult Session::make_move(int player_id, int x, int y) {
    std::lock_guard<std::mutex> lock(session_mutex_);
    if (player_id != turn_) {
        Logger::log("Session ", game_id_, ": ERROR - player ", player_id, 
            " tried to move but it's player ", turn_, "'s turn");
        return ShotResult::INVALID;
    }
    
    if (game_status_ != GameStatus::IN_PROGRESS) {
        Logger::log("Session ", game_id_, ": ERROR - player ", player_id, 
                    " tried to move but game is not in progress (status=", 
                    game_status_to_string(game_status_), ")");
        return ShotResult::INVALID;
    }
    
    auto& opponent_field = get_opponent_field(player_id);
    Logger::log("Session ", game_id_, ": player ", player_id, " making move at (", x, ",", y, ")");
    ShotResult result = opponent_field.check_shot(x, y);
    
    if (result == ShotResult::MISS) {
        if (player_id == first_player_->get_id())
            turn_ = second_player_->get_id(); 
        else 
            turn_ = first_player_->get_id();
        turn_start_time_ = std::chrono::steady_clock::now();
        Logger::log("Session ", game_id_, ": MISS, turn passes to player ", turn_);
    } else if (result == ShotResult::HIT) {
        if (opponent_field.game_is_over()) {
            winner_id_ = player_id;
            result = ShotResult::GAME_OVER;
            game_status_ = GameStatus::FINISHED;
            Logger::log("Session ", game_id_, ": HIT and game over. Winner: player ", winner_id_);
        } else {
            Logger::log("Session ", game_id_, ": HIT, player ", player_id, " shoots again");
        }
        turn_start_time_ = std::chrono::steady_clock::now();
    } else if (result == ShotResult::RESHOT) {
        Logger::log("Session ", game_id_, ": RESHOT, player ", player_id, " shoots again");
    }
    
    return result;
}

bool Session::check_timeout() {
    std::lock_guard<std::mutex> lock(session_mutex_);
    if (game_status_ != GameStatus::IN_PROGRESS) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - turn_start_time_;
    
    if (elapsed > turn_timeout_) {
        if (turn_ == first_player_->get_id())
            winner_id_ = second_player_->get_id();
        else
            winner_id_ = first_player_->get_id();
        game_status_ = GameStatus::FINISHED;
        return true;
    }
    
    return false;
}

bool Session::set_player_ready(int player_id) {
    std::lock_guard<std::mutex> lock(session_mutex_);
    if (first_player_->get_id() == player_id && !first_ready_) {
        if (!first_field_.is_ready()) {
            Logger::log("Session ", game_id_, ": player ", player_id, " tried to set ready but field is not ready");
            return false;
        }
        first_player_->set_status(PlayerStatus::READY);
        first_ready_ = true;
        Logger::log("Session ", game_id_, ": player ", player_id, " is ready");
    } else if (player_id == second_player_->get_id() && !second_ready_) {
        if (!second_field_.is_ready()) {
            Logger::log("Session ", game_id_, ": player ", player_id, " tried to set ready but field is not ready");
            return false;
        }
        second_player_->set_status(PlayerStatus::READY);
        second_ready_ = true;
        Logger::log("Session ", game_id_, ": player ", player_id, " is ready");
    }

    if (first_ready_ && second_ready_) {
        game_status_ = GameStatus::IN_PROGRESS;
        Logger::log("Session ", game_id_, ": both players ready. Game started! First turn: player ", turn_);
        turn_start_time_ = std::chrono::steady_clock::now();
    }
    return true;
}

std::pair<int, int> Session::calculate_ratings() const {
    int p1_old = first_player_->get_points();
    int p2_old = second_player_->get_points();
    
    int diff = std::max(10, std::abs(p1_old - p2_old) * 10 / 100);

    if (winner_id_ == first_player_->get_id()) {
        return { p1_old + diff, std::max(0, p2_old - diff) };
    } else {
        return { std::max(0, p1_old - diff), p2_old + diff };
    }
}

void Session::apply_new_ratings(int first_rating_new, int second_rating_new) {
    first_player_->set_points(first_rating_new);
    second_player_->set_points(second_rating_new);
}

void Session::end_game() {
    std::lock_guard<std::mutex> lock(session_mutex_);
    if (game_status_ != GameStatus::FINISHED) {
        Logger::log("Session ", game_id_, ": end_game called but game not finished");
        return;
    }

    auto [r1, r2] = calculate_ratings();
    apply_new_ratings(r1, r2);

    Logger::log("Session ", game_id_, ": player ", first_player_->get_id(),
                " new rating: ", first_player_->get_points());
    Logger::log("Session ", game_id_, ": player ", second_player_->get_id(),
                " new rating: ", second_player_->get_points());
}

std::pair<int, int> Session::get_player_ids() const {
    return std::pair<int, int> (first_player_->get_id(), second_player_->get_id());
}

int Session::get_game_id() const {
    return game_id_;
}

bool Session::game_is_over() const {
    return game_status_ == GameStatus::FINISHED;
}

int Session::get_winner_id() const {
    return winner_id_;
}

int Session::get_current_turn() const {
    return turn_;
}

GameField& Session::get_player_field(int player_id) {
    if (player_id == first_player_->get_id()) {
        return first_field_;
    }
    return second_field_;
}

GameField& Session::get_opponent_field(int player_id) {
    if (player_id == first_player_->get_id()) {
        return second_field_;
    }
    return first_field_;
}

const GameField& Session::get_first_field() const { 
    return first_field_; 
}

const GameField& Session::get_second_field() const { 
    return second_field_; 
}

const std::string& Session::get_first_login() const { 
    return first_player_->get_login(); 
}

const std::string& Session::get_second_login() const { 
    return second_player_->get_login(); 
}

bool Session::game_is_started() const {
    return game_status_ == GameStatus::IN_PROGRESS;
}
