#include "network/commands/sign_up_command.hpp"

SignUpCommand::SignUpCommand(AuthManager& am) : auth_manager_(am) {}

SignUpCommand::~SignUpCommand() = default;

void SignUpCommand::handle(const std::vector<std::string>& args, int player_id) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, player_id);
        return;
    }

    if (args.size() < 3) return;

    auth_manager_.register_user(args[1], args[2]).then([](auto fut) {
        if (fut.get()) Logger::log("New user registered");
    });
}
