#include "network/commands/ready_command.hpp"
#include "logger/logger.hpp"

ReadyCommand::ReadyCommand(SessionManager& sm) : session_manager_(sm) {}

ReadyCommand::~ReadyCommand() = default;

void ReadyCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (!context.current_player) {
        context.send_response("ERROR: Not authorized\n");
        Logger::log("PlaceShipCommand failed: anauthorized");
        return;
    }
    int player_id = context.current_player->get_id();
    int game_id = session_manager_.get_player_game(player_id);
    if (game_id == -1) {
        context.send_response("ERROR: Not authorized\n");
        Logger::log("StartGameCommand failed: anauthorized");
        return;
    }
    session_manager_.set_player_ready(player_id, game_id);
    context.send_response("Game is starting, wait...\n");
    Logger::log("Player ", player_id, " is ready in game ", game_id);
}
