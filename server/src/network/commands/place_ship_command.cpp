#include "network/commands/place_ship_command.hpp"
#include "logger/logger.hpp"

PlaceShipCommand::PlaceShipCommand(SessionManager& sm) : session_manager_(sm) {}

PlaceShipCommand::~PlaceShipCommand() = default;

void PlaceShipCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (args.size() < 5) {
        context.send_response("Missing login or password, try again");
        Logger::log("PlaceShipCommand: missing login or password");
        return;
    }

    if (!std::all_of(args[1].begin(), args[1].end(), ::isdigit) ||
        !std::all_of(args[2].begin(), args[2].end(), ::isdigit) ||
        !std::all_of(args[3].begin(), args[3].end(), ::isdigit)) {
        Logger::log("PlaceShip: arguments are not digits");
        context.send_response("Coordinates or length is not digit, try again");
        return;
    }

    int len = std::stoi(args[1]);
    int x = std::stoi(args[2]);
    int y = std::stoi(args[3]);
    bool horizontal = (args[4] == "1");

    if (!context.current_player) {
        context.send_response("ERROR: Not authorized");
        Logger::log("PlaceShipCommand failed: anauthorized");
        return;
    }
    int player_id = context.current_player->get_id();
    int game_id = session_manager_.get_player_game(player_id);
    if (game_id == -1) {
        context.send_response("ERROR: game did not start");
        Logger::log("PlaceShipCommand failed: game did not start");
        return;
    }

    session_manager_.place_ship_async(player_id, game_id, len, x, y, horizontal, [context](bool result) {
        if (result) {
            context.send_response("Ship successfully placed");
            Logger::log("Player ", context.current_player->get_id(), " placed ship");
        } else {
            context.send_response("Failed to place ship: Invalid ship placement");
            Logger::log("Player ", context.current_player->get_id(), " failed to place ship");
        }
    });
}
