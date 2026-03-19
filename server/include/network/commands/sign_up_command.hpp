#pragma once

#include "base_command.hpp"
#include "auth/auth_manager.hpp"
#include "logger/logger.hpp"

class SignUpCommand : public BaseCommand {
public:
    SignUpCommand(AuthManager& am);

    ~SignUp();

    void handle(const std::vector<std::string>& args, int player_id) override;

private:
    const std::string name_ = "signup";
    AuthManager& auth_manager_;
};
