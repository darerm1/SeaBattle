#include "network/commands/disconnect_command.hpp"
#include "logger/logger.hpp"
#include "core/player.hpp"

DisconnectCommand::DisconnectCommand() = default;

DisconnectCommand::~DisconnectCommand() = default;

void DisconnectCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (context.current_player) {
        Logger::log("Player ", std::to_string(context.current_player->get_id()), " requested disconnect");
    } else {
        Logger::log("Unauthorized connection requested disconnect");
    }
    context.send_response("GOODBYE\n");
    
    context.close_connection();
}
