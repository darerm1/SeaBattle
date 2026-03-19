#include "network/network_manager.hpp"
#include "logger/logger.hpp"

NetworkManager::NetworkManager(boost::asio::io_context& io_context, int port, SessionManager& sm, AuthManager& am)
                            : context_(io_context), 
                            acceptor_(context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
                            session_manager_(sm),
                            auth_manager_(am) {}

NetworkManager::~NetworkManager() = default;

void NetworkManager::start_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (ec) {
                Logger::log("Accept error: ", ec.message());
                start_accept();
                return;
            }
            int connection_id;
            if (connections_.empty()) connection_id = 0;
            else connection_id = connections_[connections_.size()-1]->get_connection_id() + 1;
            
            auto connection = std::make_shared<Connection>(connection_id, std::move(socket), session_manager_, *this);
            connections_[connection_id] = connection;
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
