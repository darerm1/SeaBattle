#include "network/commands/clear_field_command.hpp"

ClearFieldCommand::ClearFieldCommand(SessionManager& sm) : session_manager_(sm) {}

ClearFieldCommand::~ClearFieldCommand() = default;

void ClearFieldCommand::handle(const std::vector<std::string>& args, int player_id) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, player_id);
        return;
    }

    int game_id = session_manager_.get_player_game(player_id);
    if (game_id != -1) {
        session_manager_.clear_field(player_id, game_id);
    }
}
