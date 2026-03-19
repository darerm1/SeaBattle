#include "network/commands/login_command.hpp"

LoginCommand::LoginCommand(AuthManager& am) : auth_manager_(am) {}

LoginCommand::~LoginCommand() = default;

void LoginCommand::handle(const std::vector<std::string>& args, int player_id) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, player_id);
        return;
    }

    if (args.size() < 3) {
        Logger::log("LoginCommand: missing login or password");
        return;
    }

    std::string login = args[1];
    std::string password = args[2];

    auth_manager_.authenticate(login, password).then([](auto fut) {
        auto player = fut.get();
        if (player) {
            Logger::log("Player ", player->get_id(), " logged in");
        }
    });
}
