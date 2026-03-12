#pragma once

#include "player.hpp"
#include <vector>
#include <memory>
#include <mutex>

class Queue {
public:
    Queue();

    ~Queue();

    void push_back(std::shared_ptr<Player> player);

    void remove(int player_id);

    std::pair<std::shared_ptr<Player>, std::shared_ptr<Player>> make_game_pair();

    bool is_empty() const;

    int size() const;

private:
    std::shared_ptr<Player> find_opponent(std::shared_ptr<Player> player);

    std::vector<std::shared_ptr<Player>> queue_;
    std::mutex queue_mutex_;
};
