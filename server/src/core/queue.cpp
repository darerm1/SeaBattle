#include "core/queue.hpp"
#include <algorithm>
#include <limits>
#include <cmath>

Queue::Queue() = default;

Queue::~Queue() = default;

void Queue::push_back(std::shared_ptr<Player> player) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push_back(player);
    player->set_status(PlayerStatus::WAITING);
}

void Queue::remove(int player_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    auto it = std::find_if(queue_.begin(), queue_.end(), [player_id](const auto& p) { return p->get_id() == player_id; });
    
    if (it != queue_.end()) {
        (*it)->set_status(PlayerStatus::OFFLINE);
        queue_.erase(it);
    }
}

std::shared_ptr<Player> Queue::find_opponent(std::shared_ptr<Player> player) {
    if (queue_.empty())
        return nullptr;
    int min = std::numeric_limits<int>::max();
    int curr;
    int ind = -1;
    for (size_t i = 0; i < queue_.size(); ++i) {
        if (queue_[i] == player) continue;
        curr = std::abs(queue_[i]->get_points() - player->get_points());
        if (curr < min) {
            min = curr;
            ind = i;
        }
    }

    if (ind != -1) {
        std::shared_ptr<Player> opponent = queue_[ind];
        queue_.erase(queue_.begin() + ind);
        return opponent;
    }
    return nullptr;
}

bool Queue::is_empty() const {
    return queue_.empty();
}

int Queue::size() const {
    return queue_.size();
}

std::pair<std::shared_ptr<Player>, std::shared_ptr<Player>> Queue::make_game_pair() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_.empty())
        return std::make_pair(nullptr, nullptr);
    std::shared_ptr<Player> first_player = queue_[0];
    queue_.erase(queue_.begin());
    
    std::shared_ptr<Player> second_player = find_opponent(first_player);
    if (!second_player) {
        queue_.push_back(first_player);
        return std::make_pair(nullptr, nullptr);
    }

    return std::make_pair(first_player, second_player);
}
