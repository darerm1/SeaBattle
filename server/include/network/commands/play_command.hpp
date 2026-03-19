#pragma once

#include "base_command.hpp"
#include "network/connection.hpp"
#include "logger/logger.hpp"

class PlayCommand : public BaseCommand {
public:
    PlayCommand(SessionManager& sm);

    ~PlayCommand();

    void handle(const std::vector<std::string>& args, std::shared_ptr<Connection> connection) override;
private:
    const std::string name_ = "play";
    SessionManager& session_manager_;
};
