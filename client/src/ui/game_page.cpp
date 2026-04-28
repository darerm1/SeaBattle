#include "ui/game_page.hpp"
#include "network/client_network.hpp"
#include "ui/field_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

GamePage::GamePage(ClientNetwork* network, QWidget* parent) : QWidget(parent), network_(network) {
    renderWorker_ = new RenderWorker();
    renderWorker_->moveToThread(&renderThread_);

    connect(&renderThread_, &QThread::finished, renderWorker_, &QObject::deleteLater);
    connect(renderWorker_, &RenderWorker::updatesReady, this, &GamePage::applyUpdates);
    connect(this, &GamePage::moveResultReceived,   renderWorker_, &RenderWorker::handleMoveResult);
    connect(this, &GamePage::opponentMoveReceived, renderWorker_, &RenderWorker::handleOpponentMove);
    connect(this, &GamePage::fieldInitialized,     renderWorker_, &RenderWorker::init);

    renderThread_.start();

    setupUI();

    connect(network_, &ClientNetwork::moveResult,    this, &GamePage::onMoveResult);
    connect(network_, &ClientNetwork::opponentMove,  this, &GamePage::onOpponentMove);
    connect(network_, &ClientNetwork::gameOver,      this, &GamePage::onGameOver);
    connect(network_, &ClientNetwork::yourTurn,      this, &GamePage::onYourTurn);
    connect(network_, &ClientNetwork::gameForfeited, this, &GamePage::exitRequested);
    connect(network_, &ClientNetwork::gameInfo,      this, &GamePage::onGameInfo);
}

GamePage::~GamePage() {
    renderThread_.quit();
    renderThread_.wait();
}

void GamePage::setupUI() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* ownLayout = new QVBoxLayout();
    ownFieldLabel_ = new QLabel("Ваше поле", this);
    ownFieldLabel_->setAlignment(Qt::AlignCenter);
    ownFieldWidget_ = new FieldWidget(this);
    ownLayout->addWidget(ownFieldLabel_);
    ownLayout->addWidget(ownFieldWidget_);

    QVBoxLayout* enemyLayout = new QVBoxLayout();
    enemyFieldLabel_ = new QLabel("Поле противника", this);
    enemyFieldLabel_->setAlignment(Qt::AlignCenter);
    enemyFieldWidget_ = new FieldWidget(this);
    enemyLayout->addWidget(enemyFieldLabel_);
    enemyLayout->addWidget(enemyFieldWidget_);

    connect(enemyFieldWidget_, &FieldWidget::cellClicked, [this](int x, int y) {
        if (!myTurn_ || gameOver_) return;
        if (enemyFieldWidget_->isCellBlocked(x, y)) {
            turnLabel_->setText("Сюда уже стреляли!");
            return;
        }
        lastMoveX_ = x;
        lastMoveY_ = y;
        myTurn_ = false;
        turnLabel_->setText("Ход противника...");
        network_->sendCommand(QString("move %1 %2").arg(x).arg(y));
    });

    QVBoxLayout* infoLayout = new QVBoxLayout();
    turnLabel_ = new QLabel("Ваш ход", this);
    turnLabel_->setStyleSheet("font-weight: bold; font-size: 14px;");
    turnLabel_->setAlignment(Qt::AlignCenter);
    exitButton_ = new QPushButton("Выйти", this);

    connect(exitButton_, &QPushButton::clicked, [this]() {
        if (gameOver_) {
            emit exitRequested();
        } else {
            network_->sendCommand("forfeit");
        }
    });

    infoLayout->addWidget(turnLabel_);
    infoLayout->addSpacing(20);
    infoLayout->addStretch();
    infoLayout->addWidget(exitButton_);

    mainLayout->addLayout(ownLayout);
    mainLayout->addLayout(enemyLayout);
    mainLayout->addLayout(infoLayout);
}

void GamePage::initField(const QVector<QVector<int>>& field) {
    ownFieldWidget_->clearField();
    enemyFieldWidget_->clearField();
    myTurn_ = false;
    gameOver_ = false;
    lastMoveX_ = -1;
    lastMoveY_ = -1;
    turnLabel_->setText("Ожидание начала хода...");
    ownFieldLabel_->setText("Ваше поле");
    enemyFieldLabel_->setText("Поле противника");

    for (int y = 0; y < field.size(); ++y)
        for (int x = 0; x < field[y].size(); ++x)
            if (field[y][x] == 1)
                ownFieldWidget_->setCellState(x, y, CellState::SHIP);

    emit fieldInitialized(field);
}

void GamePage::onMoveResult(int result, int x, int y) {
    int cx = (x >= 0) ? x : lastMoveX_;
    int cy = (y >= 0) ? y : lastMoveY_;
    emit moveResultReceived(result, cx, cy);
}

void GamePage::onOpponentMove(int x, int y, int result) {
    emit opponentMoveReceived(x, y, result);
}

void GamePage::onGameOver(int winnerId) {
    Q_UNUSED(winnerId)
    gameOver_ = true;
    if (turnLabel_->text() != "Вы победили!" && turnLabel_->text() != "Вы проиграли!") {
        turnLabel_->setText("Соперник вышел из игры, вы победили");
    }
}

void GamePage::onYourTurn() {
    if (!gameOver_) {
        myTurn_ = true;
        turnLabel_->setText("Ваш ход");
    }
}

void GamePage::onGameInfo(QString ownLogin, int ownRating, QString oppLogin, int oppRating) {
    ownFieldLabel_->setText(QString("Ваше поле\n%1 (рейтинг: %2)").arg(ownLogin).arg(ownRating));
    enemyFieldLabel_->setText(QString("Поле противника\n%1 (рейтинг: %2)").arg(oppLogin).arg(oppRating));
}

void GamePage::applyUpdates(QVector<CellUpdate> updates, bool myTurn, QString label, bool gameOver) {
    for (const CellUpdate& u : updates) {
        FieldWidget* widget = u.isEnemy ? enemyFieldWidget_ : ownFieldWidget_;
        widget->setCellState(u.x, u.y, u.state);
        if (u.blocked)
            widget->setCellBlocked(u.x, u.y, true);
    }
    myTurn_ = myTurn;
    if (gameOver) gameOver_ = true;
    if (!label.isEmpty()) turnLabel_->setText(label);
}
