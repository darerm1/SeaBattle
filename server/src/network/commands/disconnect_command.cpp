#include "network/commands/disconnect_command.hpp"

DisconnectCommand::DisconnectCommand() = default;

DisconnectCommand::~DisconnectCommand() = default;

void DisconnectCommand::handle(const std::vector<std::string>& args, std::shared_ptr<Connection> connection) {
    if (args.empty() || args[0] != name_) {
        BaseCommand::handle(args, connection);
        return;
    }

    Logger::log("Player ", connection->get_player_id(), " requested disconnect");
    connection->send("GOODBYE\n");
    
    connection->close_connection();
}
