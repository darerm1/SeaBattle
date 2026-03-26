#pragma once

#include "base_command.hpp"
#include "auth/auth_manager.hpp"

class SignUpCommand : public BaseCommand {
public:
    SignUpCommand(AuthManager& am);

    ~SignUpCommand();

    void handle(const std::vector<std::string>& args, CommandContext context) override;

private:
    const std::string name_ = "signup";
    AuthManager& auth_manager_;
};
