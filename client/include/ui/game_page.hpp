#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "field_widget.hpp"

class ClientNetwork;

class GamePage : public QWidget {
    Q_OBJECT

public:
    explicit GamePage(ClientNetwork* network, QWidget* parent = nullptr);

    void initField(const QVector<QVector<int>>& field);

signals:
    void exitRequested();

private slots:
    void onMoveResult(int result, int x, int y);
    void onOpponentMove(int x, int y, int result);
    void onGameOver(int winnerId);
    void onYourTurn();
    void onGameInfo(QString ownLogin, int ownRating, QString oppLogin, int oppRating);

private:
    friend class TestGamePage;

    void setupUI();

    ClientNetwork* network_;

    FieldWidget* ownFieldWidget_;
    FieldWidget* enemyFieldWidget_;
    QLabel* ownFieldLabel_;
    QLabel* enemyFieldLabel_;
    QLabel* turnLabel_;
    QPushButton* exitButton_;

    int lastMoveX_ = -1;
    int lastMoveY_ = -1;
    bool myTurn_ = true;
    bool gameOver_ = false;
};
