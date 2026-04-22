#include "database/database_manager.hpp"
#include "auth/auth_manager.hpp"
#include "core/session_manager.hpp"
#include "network/network_manager.hpp"
#include "utils/thread_pool.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <network/commands/login_command.hpp>
#include <network/commands/sign_up_command.hpp>
#include <network/commands/play_command.hpp>
#include <network/commands/move_command.hpp>
#include <network/commands/place_ship_command.hpp>
#include <network/commands/ready_command.hpp>
#include <network/commands/disconnect_command.hpp>
#include <network/commands/clear_field_command.hpp>
#include <network/commands/forfeit_command.hpp>
#include <iostream>

int main() {
    Logger::set_verbose(true);
    const std::string db_path = "seabattle.db";

    auto db_pool = std::make_shared<ThreadPool>(4);
    auto action_pool = std::make_shared<ThreadPool>(2);

    DatabaseManager db(db_path); 
    AuthManager auth(db, *db_pool);
    SessionManager sessions(*action_pool, db);
    
    auto login = std::make_shared<LoginCommand>(auth);
    auto signup = std::make_shared<SignUpCommand>(auth);
    auto play = std::make_shared<PlayCommand>(sessions);
    auto move = std::make_shared<MoveCommand>(sessions);
    auto place = std::make_shared<PlaceShipCommand>(sessions);
    auto clear = std::make_shared<ClearFieldCommand>(sessions);
    auto ready = std::make_shared<ReadyCommand>(sessions);
    auto forfeit = std::make_shared<ForfeitCommand>(sessions);
    auto disconnect = std::make_shared<DisconnectCommand>();

    login->setNext(signup)
         ->setNext(play)
         ->setNext(move)
         ->setNext(place)
         ->setNext(clear)
         ->setNext(ready)
         ->setNext(forfeit)
         ->setNext(disconnect);
    
    boost::asio::io_context network_context;

    NetworkManager network(network_context, 8080, sessions, auth, login);

    sessions.set_notify_callback([&network](int player_id, const std::string& msg) {
        network.send_to_player(player_id, msg);
    });

    network.start_accept();
    network.run();

    return 0;
}
