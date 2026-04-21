#include "database/database_manager.hpp"
#include "auth/auth_manager.hpp"
#include "core/session_manager.hpp"
#include "network/network_manager.hpp"
#include "utils/thread_pool.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <login_command.hpp>
#include <sign_up_command.hpp>
#include <move_command.hpp>
#include <place_ship_command.hpp>
#include <ready_command.hpp>
#include <disconnect_command.hpp>
#include <clear_field_command.hpp>
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
    auto move = std::make_shared<MoveCommand>(sessions);
    auto place = std::make_shared<PlaceShipCommand>(sessions);
    auto clear = std::make_shared<ClearFieldCommand>(sessions);
    auto ready = std::make_shared<ReadyCommand>(sessions);
    auto disconnect = std::make_shared<DisconnectCommand>();
    
    login->setNext(signup)
         ->setNext(move)
         ->setNext(place)
         ->setNext(clear)
         ->setNext(ready)
         ->setNext(disconnect);
    
    boost::asio::io_context network_context;
    
    NetworkManager network(network_context, 8080, sessions, auth, login);

    network.start_accept();
    network.run();

    return 0;
}
