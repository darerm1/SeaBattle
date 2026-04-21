#pragma once

#include <boost/asio.hpp>
#include <mutex>
#include <memory>
#include "core/session_manager.hpp"
#include "core/player.hpp"
#include "auth/auth_manager.hpp"
#include "commands/base_command.hpp"

class NetworkManager; 

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(int connection_id, boost::asio::ip::tcp::socket socket, SessionManager& sm, NetworkManager& nm, AuthManager& am, std::shared_ptr<BaseCommand> command_chain);

    ~Connection();

    void start();

    void close_connection();

    void send(const std::string& message);

    void set_player(std::shared_ptr<Player> player);

    const std::shared_ptr<Player> get_player() const;

    void process_command(std::string& command);

    int get_connection_id() const;

private:
    void on_read();

    int connection_id_;
    NetworkManager& network_manager_;
    SessionManager& session_manager_;
    AuthManager& auth_manager_;
    std::shared_ptr<BaseCommand> command_chain_;
    std::shared_ptr<Player> player_ = nullptr;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf streambuf_;
};
