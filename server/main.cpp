#include "database/database_manager.hpp"
#include "auth/auth_manager.hpp"
#include "core/session_manager.hpp"
#include "network/network_manager.hpp"
#include "utils/thread_pool.hpp"
#include "network/commands/login_command.hpp"
#include "network/commands/sign_up_command.hpp"
#include "network/commands/disconnect_command.hpp"
#include "network/commands/clear_field_command.hpp"
#include "network/commands/move_command.hpp"
#include "network/commands/place_ship_command.hpp"
#include "network/commands/ready_command.hpp"
#include "network/commands/play_command.hpp"
#include "core/player.hpp"
#include <boost/asio.hpp>
#include <memory>

int main() {
    const std::string db_path = "seabattle.db";

    auto db_pool = std::make_shared<ThreadPool>(4);
    auto action_pool = std::make_shared<ThreadPool>(2);

    DatabaseManager db(db_path); 
    AuthManager am(db, *db_pool);
    SessionManager sm(*action_pool, db);
    
    auto login = std::make_shared<LoginCommand>(am); 
    auto signup = std::make_shared<SignUpCommand>(am);
    auto move = std::make_shared<MoveCommand>(sm);
    auto place = std::make_shared<PlaceShipCommand>(sm);
    auto clear = std::make_shared<ClearFieldCommand>(sm);
    auto ready = std::make_shared<ReadyCommand>(sm);
    auto disconnect = std::make_shared<DisconnectCommand>();
    
    login->setNext(signup)
         ->setNext(move)
         ->setNext(place)
         ->setNext(clear)
         ->setNext(ready)
         ->setNext(disconnect);
    
    boost::asio::io_context network_context;
    
    NetworkManager network(network_context, 8080, sm, am, login);

    network.start_accept();
    network.run();

    return 0;
}
