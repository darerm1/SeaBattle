#pragma once

#include "base_command.hpp"
#include "core/session_manager.hpp"

class ForfeitCommand : public BaseCommand {
public:
    ForfeitCommand(SessionManager& sm);
    ~ForfeitCommand();

    void handle(const std::vector<std::string>& args, CommandContext context) override;
private:
    const std::string name_ = "forfeit";
    SessionManager& session_manager_;
};
