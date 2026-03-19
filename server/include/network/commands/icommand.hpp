#pragma once

#include <memory>
#include <string>
#include <vector>

class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void handle(const std::vector<std::string>& args, int player_id) = 0;
};
