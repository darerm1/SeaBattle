#pragma once

#include "base_command.hpp"
#include "core/session_manager.hpp"

class PlaceShipCommand : public BaseCommand {
public:
    PlaceShipCommand(SessionManager& sm);

    ~PlaceShipCommand();

    void handle(const std::vector<std::string>& args, CommandContext context) override;

private:
    const std::string name_ = "place";
    SessionManager& session_manager_;
};
