#pragma once

#include <QObject>
#include <QVector>
#include <QPoint>
#include "ui/field_widget.hpp"

struct CellUpdate {
    int x = 0, y = 0;
    CellState state = CellState::EMPTY;
    bool blocked = false;
    bool isEnemy = false;
};
Q_DECLARE_METATYPE(CellUpdate)
Q_DECLARE_METATYPE(QVector<CellUpdate>)

class RenderWorker : public QObject {
    Q_OBJECT

public:
    explicit RenderWorker(QObject* parent = nullptr);

public slots:
    void init(QVector<QVector<int>> field);
    void handleMoveResult(int result, int cx, int cy);
    void handleOpponentMove(int x, int y, int result);

signals:
    void updatesReady(QVector<CellUpdate> updates, bool myTurn, QString label, bool gameOver);

private:
    void collectSunkUpdates(int x, int y, bool isEnemy, QVector<CellUpdate>& updates);

    CellState enemyCells_[10][10] = {};
    CellState ownCells_[10][10] = {};
};
