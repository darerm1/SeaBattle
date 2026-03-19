#pragma once

#include "base_command.hpp"
#include "core/session_manager.hpp"

class PlaceShipCommand : public BaseCommand {
public:
    PlaceShipCommand(SessionManager& sm);

    ~PlaceShipCommand();

    void handle(const std::vector<std::string>& args, int player_id) override;

private:
    const std::string name_ = "place";
    SessionManager& session_manager_;
};
