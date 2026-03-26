#include "network/commands/play_command.hpp"
#include "logger/logger.hpp"

PlayCommand::PlayCommand(SessionManager& sm) : session_manager_(sm) {}

PlayCommand::~PlayCommand() = default;

void PlayCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (!context.current_player) {
        context.send_response("ERROR: Not authorized");
        Logger::log("PlayCommand failed: player anauthorized");
        return;
    }
    
    session_manager_.add_to_queue_async(context.current_player);
    context.send_response("Searching opponent...");
    Logger::log("Player ", context.current_player->get_id(), " joined the queue");
}
