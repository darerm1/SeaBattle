#pragma once

#include "base_command.hpp"
#include "core/session_manager.hpp"

class ClearFieldCommand : public BaseCommand {
public:
    ClearFieldCommand(SessionManager& sm);

    ~ClearFieldCommand();

    void handle(const std::vector<std::string>& args, CommandContext context) override;

private:
    const std::string name_ = "clear";
    SessionManager& session_manager_;
};
