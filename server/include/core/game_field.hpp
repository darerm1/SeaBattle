#pragma once

#include <vector>
#include "shot_result.hpp"
#include "cell_state.hpp"

static constexpr int FIELD_SIZE = 10;

class GameField {
public:
    GameField();

    ~GameField();

    bool set_ship(int length, int x, int y, bool is_horizontal);

    void clear_field();

    ShotResult check_shot(int x, int y);

    bool game_is_over() const;

    const std::vector<std::vector<CellState>>& get_cells() const;

    bool is_ready() const;

    bool is_ship_sunk_at(int x, int y) const;

private:
    std::vector<std::vector<CellState>> field_;
    std::vector<int> ships_ {0, 4, 3, 2, 1 };
};
