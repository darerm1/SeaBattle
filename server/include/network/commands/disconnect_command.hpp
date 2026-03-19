#pragma once

#include "base_command.hpp"
#include "logger/logger.hpp"
#include "connection.hpp"

class DisconnectCommand : public BaseCommand {
public:
    DisconnectCommand();

    ~DisconnectCommand();

    void handle(const std::vector<std::string>& args, std::shared_ptr<Connection> connection) override;
private:
    const std::string name_ = "disconnect";
};
