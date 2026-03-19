#pragma once

#include <unordered_map>
#include <mutex>
#include "connection.hpp"
#include "core/session_manager.hpp"
#include "auth/auth_manager.hpp"

class NetworkManager {
public:
    NetworkManager(boost::asio::io_context& io_context, int port, SessionManager& sm, AuthManager& am);

    ~NetworkManager();

    void start_accept(); 

    void run();

    void remove_connection(int connection_id);

private:
    boost::asio::io_context& context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    SessionManager& session_manager_;
    AuthManager& auth_manager_;
    std::unordered_map<int, std::shared_ptr<Connection>> connections_;
    std::mutex connections_mutex_;
};
