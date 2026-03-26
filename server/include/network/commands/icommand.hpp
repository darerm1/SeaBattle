#pragma once

#include "../command_context.hpp"
#include <memory>
#include <string>
#include <vector>

class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void handle(const std::vector<std::string>& args, CommandContext context) = 0;
};
