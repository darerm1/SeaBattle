#pragma once

#include "base_command.hpp"
#include "auth/auth_manager.hpp"

class LoginCommand : public BaseCommand {
public:
    LoginCommand(AuthManager& am);

    ~LoginCommand();

    void handle(const std::vector<std::string>& args, CommandContext context) override;
private:
    const std::string name_ = "login";
    AuthManager& auth_manager_;
};
