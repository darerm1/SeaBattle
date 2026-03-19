#include "network/connection.hpp"
#include "network/network_manager.hpp" 
#include "logger/logger.hpp"
#include "core/player.hpp"
#include <boost/system/error_code.hpp>

Connection::Connection(int connection_id, boost::asio::ip::tcp::socket socket, SessionManager& sm, NetworkManager& nm, AuthManager& am, std::shared_ptr<BaseCommand> command_chain) 
                        : connection_id_(connection_id), 
                        socket_(std::move(socket)), 
                        session_manager_(sm), 
                        network_manager_(nm),
                        auth_manager_(am),
                        command_chain_(command_chain) {}

Connection::~Connection() = default;

void Connection::start() {
    on_read();
}

void Connection::on_read() {
    boost::asio::async_read_until(socket_, streambuf_, '\n', 
        [self = shared_from_this()](boost::system::error_code ec, size_t bytes) {
            if (ec) {
                self->close_connection();
                return;
            }
            std::istream is(&self->streambuf_);
            std::string command;
            std::getline(is, command);
            self->process_command(command);
            self->on_read();
        });
}

void Connection::close_connection() {
    boost::system::error_code ec;
    socket_.close(ec);
    if (player_id_ != -1) {
        session_manager_.player_disconnected(player_id_);
    }
    network_manager_.remove_connection(connection_id_);
}

void Connection::send(const std::string& message) {
    boost::asio::async_write(socket_, boost::asio::buffer(message),
        [self = shared_from_this()](boost::system::error_code ec, size_t) {
            if (ec) self->close_connection();
        });
}

void Connection::set_player_id(int player_id) {
    player_id_ = player_id;
}

const int Connection::get_player_id() const {
    return player_id_;
}


int Connection::get_connection_id() const {
    return connection_id_;
}

void Connection::process_command(std::string& command) {
    std::vector<std::string> args;
    std::istringstream iss(command);
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    if (!args.empty() && command_chain_) {
        command_chain_->handle(args, player_id_);
    }
}

