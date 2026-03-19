#include "network/commands/move_command.hpp"

MoveCommand::MoveCommand(SessionManager& sm) : session_manager_(sm) {}

MoveCommand::~MoveCommand() = default;

void MoveCommand::handle(const std::vector<std::string>& args, int player_id) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, player_id);
        return;
    }

    if (args.size() < 3) {
        Logger::log("MoveCommand: missing coordinates");
        return;
    }

    if (!std::all_of(args[1].begin(), args[1].end(), ::isdigit) ||
        !std::all_of(args[2].begin(), args[2].end(), ::isdigit)) {
        Logger::log("MoveCommand: arguments are not digits");
        return;
    }

    int x = std::stoi(args[1]);
    int y = std::stoi(args[2]);

    int game_id = session_manager_.get_player_game(player_id);
    session_manager_.make_move(player_id, game_id, x, y);
}
