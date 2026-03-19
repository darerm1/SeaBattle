#include "network/commands/place_ship_command.hpp"

PlaceShipCommand::PlaceShipCommand(SessionManager& sm) : session_manager_(sm) {}

PlaceShipCommand::~PlaceShipCommand() = default;

void PlaceShipCommand::handle(const std::vector<std::string>& args, int player_id) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, player_id);
        return;
    }

    if (args.size() < 5) return;

    int len = std::stoi(args[1]);
    int x = std::stoi(args[2]);
    int y = std::stoi(args[3]);
    bool horizontal = (args[4] == "1");

    int game_id = session_manager_.get_player_game(player_id);
    if (game_id != -1) {
        session_manager_.place_ship(player_id, game_id, len, x, y, horizontal);
    }
}
