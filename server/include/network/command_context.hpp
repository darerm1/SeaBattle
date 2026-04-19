#pragma once

#include <functional>
#include <memory>
#include <string>

class Player;

struct CommandContext {
    int connection_id;
    std::shared_ptr<Player> current_player;

    std::function<void(const std::string&)> send_response;

    std::function<void(std::shared_ptr<Player>)> on_login_success;

    std::function<void()> close_connection;
};
