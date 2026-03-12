#pragma once

#include <string>
#include "player_status.hpp"

class Player {
public:
    Player(int id, std::string login, std::string password);

    ~Player();

    bool check_password(std::string password) const;

    int get_points() const;

    void set_points(int points);

    PlayerStatus get_status() const;
    
    void set_status(PlayerStatus status);
    
    int get_id() const;
    
    const std::string& get_login() const;

private:
    int id_;
    std::string login_;
    std::string password_;
    int points_;
    PlayerStatus player_status_;
};
