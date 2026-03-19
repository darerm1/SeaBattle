#include "network/commands/start_game_command.hpp"

ReadyCommand::ReadyCommand(SessionManager& sm) : session_manager_(sm) {}

ReadyCommand::~ReadyCommand() = default;

void ReadyCommand::handle(const std::vector<std::string>& args, int player_id) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, player_id);
        return;
    }

    int game_id = session_manager_.get_player_game(player_id);
    if (game_id != -1) {
        session_manager_.set_player_ready(player_id, game_id);
        Logger::log("Player ", player_id, " is ready in game ", game_id);
    }
}
