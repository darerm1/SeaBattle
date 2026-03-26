#include "network/commands/clear_field_command.hpp"
#include "logger/logger.hpp"

ClearFieldCommand::ClearFieldCommand(SessionManager& sm) : session_manager_(sm) {}

ClearFieldCommand::~ClearFieldCommand() = default;

void ClearFieldCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (!context.current_player) {
        context.send_response("ERROR: Not authorized");
        Logger::log("ClearField failed: anauthorized");
        return;
    }
    int game_id = session_manager_.get_player_game(context.current_player->get_id());
    if (game_id == -1) {
        context.send_response("ERROR: game did not start");
        Logger::log("ClearField failed: game did not start");
        return;
    }
    
    session_manager_.clear_field_async(context.current_player->get_id(), game_id, [context](bool success) {
        if (success) {
            context.send_response("FIELD_CLEARED");
            Logger::log("ClearFieldCommand: Field cleared for player ", context.current_player->get_id());
        } else {
            context.send_response("ERROR: Could not clear field");
            Logger::log("ClearFieldCommand: Failed to clear field for player ", context.current_player->get_id());
        }
    });
    
}
