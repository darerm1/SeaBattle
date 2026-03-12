#pragma once

#include "player.hpp"
#include <vector>
#include <memory>

class Queue {
public:
    Queue();

    ~Queue();

    void push_back(std::shared_ptr<Player> player);

    void remove(int player_id);

    std::shared_ptr<Player> pop_front();

    std::shared_ptr<Player> find_opponent(std::shared_ptr<Player> player);

    bool is_empty() const;

    int size() const;

private:
    std::vector<std::shared_ptr<Player>> queue_;
};
