#pragma once

#include "base_command.hpp"
#include "core/session_manager.hpp"
#include "logger/logger.hpp"

class ReadyCommand : public BaseCommand {
public:
    ReadyCommand(SessionManager& sm);

    ~ReadyCommand();

    void handle(const std::vector<std::string>& args, int player_id) override;

private:
    const std::string name_ = "ready";
    SessionManager& session_manager_;
};
