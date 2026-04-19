#include "network/commands/move_command.hpp"
#include "logger/logger.hpp"

MoveCommand::MoveCommand(SessionManager& sm) : session_manager_(sm) {}

MoveCommand::~MoveCommand() = default;

void MoveCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, context);
        return;
    }

    if (args.size() < 3) {
        context.send_response("Missing login or password, try again");
        Logger::log("MoveCommand: missing coordinates");
        return;
    }

    if (!std::all_of(args[1].begin(), args[1].end(), ::isdigit) ||
        !std::all_of(args[2].begin(), args[2].end(), ::isdigit)) {
        context.send_response("Coordinats are not digits, try again");
        Logger::log("MoveCommand: arguments are not digits");
        return;
    }

    int x = std::stoi(args[1]);
    int y = std::stoi(args[2]);

    if (!context.current_player) {
        context.send_response("ERROR: Not authorized");
        Logger::log("MoveCommand failed: anauthorized");
        return;
    }
    int game_id = session_manager_.get_player_game(context.current_player->get_id());
    if (game_id == -1) {
        context.send_response("ERROR: game did not start");
        Logger::log("MoveCommand failed: game did not start");
        return;
    }

    session_manager_.make_move_async(context.current_player->get_id(), game_id, x, y, [context, x, y](ShotResult result) {
        context.send_response("Move result: " + std::to_string(static_cast<int>(result)));
        Logger::log("Player ", context.current_player->get_id(), " moved to (", x, ", ", y, "). Shot result: ", shot_result_to_string(result));
    });
}
