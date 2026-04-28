#include "core/game_field.hpp"

GameField::GameField() {
    field_.resize(FIELD_SIZE, std::vector<CellState>(FIELD_SIZE, CellState::EMPTY));
}

GameField::~GameField() = default;

bool GameField::set_ship(int length, int x, int y, bool is_horizontal) {
    if (length < 1 || length > 4) return false;
    if (ships_[length] == 0) return false;
    if (x < 0 || x >= FIELD_SIZE || y < 0 || y >= FIELD_SIZE) {
        return false;
    }

    int x1, x2, y1, y2;
    x1 = std::max(0, x-1);
    y1 = std::max(0, y-1);

    if (is_horizontal) {
        if (x + length - 1 >= FIELD_SIZE) return false; 
        x2 = std::min(FIELD_SIZE-1, x+length);
        y2 = std::min(FIELD_SIZE-1, y+1);
        
        for (int i = x; i < x + length; ++i) {
            if (field_[y][i] != CellState::EMPTY) 
                return false;
        }

        for (int i = x; i < x + length; ++i) {
            field_[y][i] = CellState::SHIP;
        }
    } else {
        if (y + length -1 >= FIELD_SIZE) return false;
        x2 = std::min(FIELD_SIZE-1, x+1);
        y2 = std::min(FIELD_SIZE-1, y+length);


        for (int i = y; i < y + length; ++i) {
            if (field_[i][x] != CellState::EMPTY)
                return false;
        }

        for (int i = y; i < y + length; ++i) {
            field_[i][x] = CellState::SHIP;
        }
    }

    for (int i = x1; i <= x2; ++i) {
        for (int j = y1; j <= y2; ++j) {
            if (field_[j][i] != CellState::SHIP) {
                field_[j][i] = CellState::BORDER;
            }
        }
    }

    --ships_[length];
    return true;
}

void GameField::clear_field() {
    for (int i = 0; i <= FIELD_SIZE-1; ++i) {
        for (int j = 0; j <= FIELD_SIZE-1; ++j) {
            field_[i][j] = CellState::EMPTY;
        }
    }
    ships_ = {0, 4, 3, 2, 1};
}

ShotResult GameField::check_shot(int x, int y) {
    if (x < 0 || x >= FIELD_SIZE || y < 0 || y >= FIELD_SIZE) {
        return ShotResult::INVALID;
    }
    
    if (field_[y][x] == CellState::SHIP) {
        field_[y][x] = CellState::HIT;
        return ShotResult::HIT;
    } else if (field_[y][x] == CellState::EMPTY || field_[y][x] == CellState::BORDER){
        field_[y][x] = CellState::MISS;
        return ShotResult::MISS;
    } else {
        return ShotResult::RESHOT;
    }
}

bool GameField::game_is_over() const {
    for (int i = 0; i <= FIELD_SIZE-1; ++i) {
        for (int j = 0; j <= FIELD_SIZE-1; ++j) {
            if (field_[i][j] == CellState::SHIP) {
                return false;
            }
        }
    }
    return true;
}

const std::vector<std::vector<CellState>>& GameField::get_cells() const {
    return field_;
}

bool GameField::is_ship_sunk_at(int x, int y) const {
    int x_start = x, x_end = x;
    while (x_start > 0 && field_[y][x_start-1] == CellState::HIT) x_start--;
    while (x_end < FIELD_SIZE-1 && field_[y][x_end+1] == CellState::HIT) x_end++;

    int y_start = y, y_end = y;
    while (y_start > 0 && field_[y_start-1][x] == CellState::HIT) y_start--;
    while (y_end < FIELD_SIZE-1 && field_[y_end+1][x] == CellState::HIT) y_end++;

    if (x_start > 0 && field_[y][x_start-1] == CellState::SHIP) return false;
    if (x_end < FIELD_SIZE-1 && field_[y][x_end+1] == CellState::SHIP) return false;
    if (y_start > 0 && field_[y_start-1][x] == CellState::SHIP) return false;
    if (y_end < FIELD_SIZE-1 && field_[y_end+1][x] == CellState::SHIP) return false;
    return true;
}

bool GameField::is_ready() const {
    for (int i = 1; i <= 4; ++i) {
        if (ships_[i] != 0) return false;
    }
    return true;
}
