#include "network/commands/sign_up_command.hpp"
#include "logger/logger.hpp"

SignUpCommand::SignUpCommand(AuthManager& am) : auth_manager_(am) {}

SignUpCommand::~SignUpCommand() = default;

void SignUpCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (args.size() < 3) {
        context.send_response("Missing login or password, try again\n");
        Logger::log("LoginCommand: missing login or password");
        return;
    }

    auth_manager_.register_user_async(args[1], args[2], [context](std::shared_ptr<Player> player) {
        if (player) {
            context.on_login_success(player);
            context.send_response("New user registered\n");
            Logger::log("Player ", player->get_id(), " successfully created and logged in");
        } else {
            Logger::log("Registration failed");
            context.send_response("Reqistration failed\n");
        }
    });
}
