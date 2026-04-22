#include "network/network_manager.hpp"
#include "logger/logger.hpp"

NetworkManager::NetworkManager(boost::asio::io_context& io_context, int port, SessionManager& sm, AuthManager& am, std::shared_ptr<BaseCommand> chain)
                            : context_(io_context), 
                            acceptor_(context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
                            session_manager_(sm),
                            auth_manager_(am),
                            command_chain_(chain) {}

NetworkManager::~NetworkManager() = default;

void NetworkManager::start_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (ec) {
                Logger::log("Accept error: ", ec.message());
                start_accept();
                return;
            }
            int connection_id = next_connection_id_.fetch_add(1);
            auto connection = std::make_shared<Connection>(connection_id, std::move(socket), session_manager_, *this, auth_manager_, command_chain_);
            {
            std::lock_guard<std::mutex> lock(connections_mutex_);
                connections_[connection_id] = connection;
            }
            connection->start();
        
            start_accept();
        });
}

void NetworkManager::run() {
    context_.run();
}

void NetworkManager::remove_connection(int connection_id) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(connection_id);
}

void NetworkManager::send_to_player(int player_id, const std::string& message) {
    Logger::log("send_to_player: called for player ", player_id);
    std::shared_ptr<Connection> target;
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        Logger::log("send_to_player: searching for player ", player_id, " among ", connections_.size(), " connections");
        for (auto& [id, conn] : connections_) {
            auto player = conn->get_player();
            Logger::log("  connection ", id, " -> player ", (player ? std::to_string(player->get_id()) : "none"));
            if (player && player->get_id() == player_id) {
                Logger::log("send_to_player: found connection for player ", player_id);
                target = conn;
                break;
            }
        }
    }
    if (target) {
        Logger::log("send_to_player: posting message to player ", player_id);
        auto msg = std::make_shared<std::string>(message);
        boost::asio::post(context_, [target, msg]() {
            target->send(*msg);
        });
    } else {
        Logger::log("send_to_player: player ", player_id, " NOT FOUND");
    }
}
