#include "ui/game_page.hpp"
#include "network/client_network.hpp"
#include "ui/field_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

GamePage::GamePage(ClientNetwork* network, QWidget* parent)
    : QWidget(parent)
    , network_(network)
{
    setupUI();
    
    connect(network_, &ClientNetwork::moveResult, this, &GamePage::onMoveResult);
    connect(network_, &ClientNetwork::gameOver, this, &GamePage::onGameOver);
}

void GamePage::setupUI() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    
    ownFieldWidget_ = new FieldWidget(this);
    enemyFieldWidget_ = new FieldWidget(this);
    
    connect(enemyFieldWidget_, &FieldWidget::cellClicked, [this](int x, int y) {
        network_->sendCommand(QString("move %1 %2").arg(x).arg(y));
    });
    
    QVBoxLayout* infoLayout = new QVBoxLayout();
    turnLabel_ = new QLabel("Ваш ход", this);
    ownShipsLabel_ = new QLabel("Корабли: 10", this);
    enemyShipsLabel_ = new QLabel("Корабли: 10", this);
    exitButton_ = new QPushButton("Выйти", this);
    
    connect(exitButton_, &QPushButton::clicked, [this]() {
        network_->sendCommand("disconnect");
    });
    
    infoLayout->addWidget(turnLabel_);
    infoLayout->addWidget(ownShipsLabel_);
    infoLayout->addWidget(enemyShipsLabel_);
    infoLayout->addWidget(exitButton_);
    infoLayout->addStretch();
    
    mainLayout->addWidget(ownFieldWidget_);
    mainLayout->addWidget(enemyFieldWidget_);
    mainLayout->addLayout(infoLayout);
}

void GamePage::onMoveResult(int result, int x, int y) {
    if (x >= 0 && y >= 0) {
        if (result == 0) {
            enemyFieldWidget_->setCellState(x, y, CellState::HIT);
        } else if (result == 1) {
            enemyFieldWidget_->setCellState(x, y, CellState::MISS);
        }
    }
    if (result == 3) {
        turnLabel_->setText("Игра окончена!");
    }
}

void GamePage::onGameOver(int winnerId) {
    turnLabel_->setText(QString("Победитель: Игрок %1").arg(winnerId));
}
