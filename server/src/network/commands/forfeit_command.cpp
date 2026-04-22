#include "network/commands/forfeit_command.hpp"
#include "logger/logger.hpp"

ForfeitCommand::ForfeitCommand(SessionManager& sm) : session_manager_(sm) {}

ForfeitCommand::~ForfeitCommand() = default;

void ForfeitCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (!context.current_player) {
        context.send_response("ERROR: Not authorized\n");
        return;
    }

    int player_id = context.current_player->get_id();
    int game_id = session_manager_.get_player_game(player_id);
    if (game_id == -1) {
        context.send_response("ERROR: Not in a game\n");
        return;
    }

    Logger::log("Player ", player_id, " forfeited game ", game_id);
    session_manager_.forfeit_game_async(player_id, game_id, [context]() {
        context.send_response("Forfeited\n");
    });
}
