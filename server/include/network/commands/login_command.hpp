#pragma once

#include "base_command.hpp"
#include "auth/auth_manager.hpp"
#include "logger/logger.hpp"

class LoginCommand : public BaseCommand {
public:
    LoginCommand(AuthManager& am);

    ~LoginCommand();

    void handle(const std::vector<std::string>& args, int player_id) override;
private:
    const std::string name_ = "login";
    AuthManager& auth_manager_;
};
