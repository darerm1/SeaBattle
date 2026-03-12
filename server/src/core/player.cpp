#include "core/player.hpp"

Player::Player(int id, std::string login, std::string password) 
    : id_(id), login_(login), password_(password), points_(0), player_status_(PlayerStatus::OFFLINE) {
}

Player::~Player() = default;

bool Player::check_password(std::string password) const {
    if (password == password_)
        return true;
    return false;
}

int Player::get_points() const {
    return points_;
}

void Player::set_points(int points) {
    points_ = points;
}

PlayerStatus Player::get_status() const {
    return player_status_;
}

void Player::set_status(PlayerStatus status) {
    player_status_ = status;
}

int Player::get_id() const {
    return id_;
}

const std::string& Player::get_login() const {
    return login_;
}
