#include "ui/render_worker.hpp"

RenderWorker::RenderWorker(QObject* parent) : QObject(parent) {
    qRegisterMetaType<QVector<CellUpdate>>();
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x) {
            enemyCells_[y][x] = CellState::EMPTY;
            ownCells_[y][x]   = CellState::EMPTY;
        }
}

void RenderWorker::init(QVector<QVector<int>> field) {
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x) {
            enemyCells_[y][x] = CellState::EMPTY;
            ownCells_[y][x]   = (y < field.size() && x < field[y].size() && field[y][x] == 1)
                                 ? CellState::SHIP : CellState::EMPTY;
        }
}

void RenderWorker::handleMoveResult(int result, int cx, int cy) {
    QVector<CellUpdate> updates;
    bool myTurn = true;
    QString label;
    bool gameOver = false;

    if (result == 0) {
        if (cx >= 0) {
            enemyCells_[cy][cx] = CellState::HIT;
            updates.append({cx, cy, CellState::HIT, true, true});
        }
        label = "Попадание! Ваш ход";
    } else if (result == 2) {
        if (cx >= 0) {
            enemyCells_[cy][cx] = CellState::HIT;
            collectSunkUpdates(cx, cy, true, updates);
        }
        label = "Корабль потоплен! Ваш ход";
    } else if (result == 1) {
        if (cx >= 0) {
            enemyCells_[cy][cx] = CellState::MISS;
            updates.append({cx, cy, CellState::MISS, true, true});
        }
        myTurn = false;
        label = "Промах. Ход противника";
    } else if (result == 3) {
        if (cx >= 0) {
            enemyCells_[cy][cx] = CellState::HIT;
            collectSunkUpdates(cx, cy, true, updates);
        }
        gameOver = true;
        label = "Вы победили!";
    }

    emit updatesReady(updates, myTurn, label, gameOver);
}

void RenderWorker::handleOpponentMove(int x, int y, int result) {
    QVector<CellUpdate> updates;
    bool myTurn = false;
    QString label;
    bool gameOver = false;

    if (result == 0) {
        ownCells_[y][x] = CellState::HIT;
        updates.append({x, y, CellState::HIT, false, false});
        label = "Противник попал! Ход противника";
    } else if (result == 2) {
        ownCells_[y][x] = CellState::HIT;
        collectSunkUpdates(x, y, false, updates);
        label = "Противник потопил ваш корабль! Ход противника";
    } else if (result == 1) {
        ownCells_[y][x] = CellState::MISS;
        updates.append({x, y, CellState::MISS, false, false});
        myTurn = true;
        label = "Противник промахнулся! Ваш ход";
    } else if (result == 3) {
        ownCells_[y][x] = CellState::HIT;
        collectSunkUpdates(x, y, false, updates);
        gameOver = true;
        label = "Вы проиграли!";
    }

    emit updatesReady(updates, myTurn, label, gameOver);
}

void RenderWorker::collectSunkUpdates(int x, int y, bool isEnemy, QVector<CellUpdate>& updates) {
    auto& field = isEnemy ? enemyCells_ : ownCells_;

    bool horizontal = (x > 0 && field[y][x - 1] == CellState::HIT)
                   || (x < 9 && field[y][x + 1] == CellState::HIT);

    QVector<QPoint> shipCells;
    if (horizontal) {
        int left = x;
        while (left > 0 && field[y][left - 1] == CellState::HIT) left--;
        int right = x;
        while (right < 9 && field[y][right + 1] == CellState::HIT) right++;
        for (int i = left; i <= right; ++i) shipCells.append(QPoint(i, y));
    } else {
        int top = y;
        while (top > 0 && field[top - 1][x] == CellState::HIT) top--;
        int bottom = y;
        while (bottom < 9 && field[bottom + 1][x] == CellState::HIT) bottom++;
        for (int i = top; i <= bottom; ++i) shipCells.append(QPoint(x, i));
    }

    for (const QPoint& p : shipCells) {
        updates.append({p.x(), p.y(), CellState::HIT, true, isEnemy});
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = p.x() + dx, ny = p.y() + dy;
                if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10) {
                    if (field[ny][nx] == CellState::EMPTY) {
                        field[ny][nx] = CellState::MISS;
                        updates.append({nx, ny, CellState::MISS, true, isEnemy});
                    }
                }
            }
        }
    }
}
