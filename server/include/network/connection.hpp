#pragma once

#include <boost/asio.hpp>
#include <mutex>
#include <memory>
#include "core/session_manager.hpp"
#include "core/player.hpp"
#include "auth/auth_manager.hpp"
#include "base_command.hpp"

class NetworkManager; 

class Connection : std::enable_shared_from_this<Connection> {
public:
    Connection(int connection_id, boost::asio::ip::tcp::socket socket, SessionManager& sm, NetworkManager& nm, AuthManager& am, std::shared_ptr<BaseCommand> command_chain);

    ~Connection();

    void start();

    void close_connection();

    void send(const std::string& message);

    void set_player_id(int player_id);

    const int get_player_id() const;

    void process_command(std::string& command);

    int get_connection_id() const;

private:
    void on_read();

    int connection_id_;
    NetworkManager& network_manager_;
    SessionManager& session_manager_;
    AuthManager& auth_manager_;
    std::shared_ptr<BaseCommand> command_chain_;
    int player_id_ = -1;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf streambuf_;
};
