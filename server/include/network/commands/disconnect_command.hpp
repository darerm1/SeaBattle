#pragma once

#include "base_command.hpp"

class DisconnectCommand : public BaseCommand {
public:
    DisconnectCommand();

    ~DisconnectCommand();

    void handle(const std::vector<std::string>& args, CommandContext context) override;
private:
    const std::string name_ = "disconnect";
};
