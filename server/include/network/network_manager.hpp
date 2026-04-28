#pragma once

#include <unordered_map>
#include <mutex>
#include <memory>
#include <atomic>
#include "connection.hpp"
#include "core/session_manager.hpp"
#include "auth/auth_manager.hpp"
#include "commands/base_command.hpp"

class NetworkManager {
public:
    NetworkManager(boost::asio::io_context& io_context, int port, SessionManager& sm, AuthManager& am, std::shared_ptr<BaseCommand> chain);

    ~NetworkManager();

    void start_accept(); 

    void run();

    void remove_connection(int connection_id);

    void send_to_player(int player_id, const std::string& message);

private:
    boost::asio::io_context& context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    SessionManager& session_manager_;
    AuthManager& auth_manager_;
    std::unordered_map<int, std::shared_ptr<Connection>> connections_;
    std::shared_ptr<BaseCommand> command_chain_;
    std::atomic<int> next_connection_id_{0};
    std::mutex connections_mutex_;
};
