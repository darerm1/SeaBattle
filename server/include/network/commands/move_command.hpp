#pragma once

#include "base_command.hpp"
#include "core/session_manager.hpp"

class MoveCommand : public BaseCommand {
public:
    MoveCommand(SessionManager& sm);

    ~MoveCommand();

    void handle(const std::vector<std::string>& args, CommandContext context) override;
private:
    SessionManager& session_manager_;
    const std::string name_ = "move";
};
