#include "network/commands/login_command.hpp"
#include "logger/logger.hpp"

LoginCommand::LoginCommand(AuthManager& am) : auth_manager_(am) {}

LoginCommand::~LoginCommand() = default;

void LoginCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (args.size() < 3) {
        context.send_response("Missing login or password, try again\n");
        Logger::log("LoginCommand: missing login or password");
        return;
    }

    std::string login = args[1];
    std::string password = args[2];

    auth_manager_.authenticate_async(login, password, [context](std::shared_ptr<Player> player) {
        if (player) {
            context.on_login_success(player);
            context.send_response("Logging successful\nRating: " + std::to_string(player->get_points()) + "\n");
            Logger::log("Player ", player->get_id(), " logged in");
        } else {
            context.send_response("Login failed. Invalid details\n");
            Logger::log("Login failed");
        }
    });
}
