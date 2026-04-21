#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <memory>
#include "field_widget.hpp"

class ClientNetwork;

class GamePage : public QWidget {
    Q_OBJECT

public:
    explicit GamePage(ClientNetwork* network, QWidget* parent = nullptr);

private slots:
    void onMoveResult(int result, int x, int y);
    void onGameOver(int winnerId);

private:
    void setupUI();
    
    ClientNetwork* network_;
    
    FieldWidget* ownFieldWidget_;
    FieldWidget* enemyFieldWidget_;
    QLabel* turnLabel_;
    QLabel* ownShipsLabel_;
    QLabel* enemyShipsLabel_;
    QPushButton* exitButton_;
};
