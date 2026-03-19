#include "network/commands/play_command.hpp"

PlayCommand::PlayCommand(SessionManager& sm) : session_manager_(sm) {}

PlayCommand::~PlayCommand() = default;

void PlayCommand::handle(const std::vector<std::string>& args, std::shared_ptr<Connection> connection) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, connection);
        return;
    }

    int player_id = connection->get_player_id();
    
    auto player = connection->get_player_ptr(); 
    
    if (player) {
        session_manager_.add_to_queue(player);
        connection->send("SEARCHING_OPPONENT\n");
        Logger::log("Player ", player_id, " joined the queue");
    }
}
