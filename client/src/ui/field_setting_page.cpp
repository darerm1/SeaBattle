#include "ui/field_setting_page.hpp"
#include "network/client_network.hpp"
#include "ui/field_widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

FieldSettingPage::FieldSettingPage(ClientNetwork* network, QWidget* parent)
    : QWidget(parent)
    , network_(network)
    , selectedShipType_(-1)
    , horizontal_(true)
{
    field_ = QVector<QVector<int>>(10, QVector<int>(10, 0));
    placedShips_ = QVector<int>(4, 0);
    setupUI();
    
    connect(network_, &ClientNetwork::gameStarted, this, &FieldSettingPage::onGameStarted);
    connect(network_, &ClientNetwork::fieldCleared, this, &FieldSettingPage::onFieldCleared);
}

void FieldSettingPage::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QLabel* titleLabel = new QLabel("Расстановка кораблей", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    mainLayout->addWidget(titleLabel);
    
    QHBoxLayout* shipPanelLayout = new QHBoxLayout();
    shipPanelLayout->addWidget(new QLabel("Выберите корабль:", this));
    
    shipButtons_.append(new QPushButton("4-клеточный", this));
    shipButtons_.append(new QPushButton("3-клеточный", this));
    shipButtons_.append(new QPushButton("2-клеточный", this));
    shipButtons_.append(new QPushButton("1-клеточный", this));
    
    for (int i = 0; i < shipButtons_.size(); ++i) {
        shipPanelLayout->addWidget(shipButtons_[i]);
        connect(shipButtons_[i], &QPushButton::clicked, [this, i]() { selectShip(i); });
    }
    
    QPushButton* rotateButton = new QPushButton("Повернуть", this);
    connect(rotateButton, &QPushButton::clicked, this, &FieldSettingPage::rotateShip);
    shipPanelLayout->addWidget(rotateButton);
    shipPanelLayout->addStretch();
    mainLayout->addLayout(shipPanelLayout);
    
    fieldWidget_ = new FieldWidget(this);
    fieldWidget_->setPlacementMode(true);
    mainLayout->addWidget(fieldWidget_, 0, Qt::AlignCenter);
    
    connect(fieldWidget_, &FieldWidget::cellClicked, this, &FieldSettingPage::onCellClicked);
    connect(fieldWidget_, &FieldWidget::mouseHover, this, &FieldSettingPage::onCellHover);
    
    statusLabel_ = new QLabel("Расставьте корабли и нажмите 'Готов'", this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet("color: blue;");
    mainLayout->addWidget(statusLabel_);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    readyButton_ = new QPushButton("Готов", this);
    clearButton_ = new QPushButton("Очистить поле", this);
    
    connect(clearButton_, &QPushButton::clicked, this, &FieldSettingPage::onClearFieldClicked);
    connect(readyButton_, &QPushButton::clicked, this, &FieldSettingPage::onReadyClicked);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(clearButton_);
    buttonLayout->addWidget(readyButton_);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    readyButton_->setEnabled(false);
    updateFieldDisplay();
}

void FieldSettingPage::selectShip(int type) {
    selectedShipType_ = type;
    statusLabel_->setText(QString("Выбран корабль: %1-клеточный").arg(4 - type));
}

void FieldSettingPage::rotateShip() {
    horizontal_ = !horizontal_;
    statusLabel_->setText(horizontal_ ? "Горизонтальная ориентация" : "Вертикальная ориентация");
}

void FieldSettingPage::onCellClicked(int x, int y) {
    if (selectedShipType_ == -1) return;
    int length = 4 - selectedShipType_;
    if (canPlaceShip(y, x, length, horizontal_)) {
        placeShipAt(y, x, length, horizontal_);
        network_->sendCommand(QString("place %1 %2 %3 %4")
                              .arg(length).arg(x).arg(y).arg(horizontal_ ? 1 : 0));
        placedShips_[selectedShipType_]++;
        selectedShipType_ = -1;
        updateFieldDisplay();
        checkReady();
        statusLabel_->setText("Корабль размещён");
        fieldWidget_->clearPreview();
    } else {
        statusLabel_->setText("Невозможно разместить корабль здесь");
    }
}

void FieldSettingPage::onCellHover(int x, int y) {
    if (selectedShipType_ == -1) return;
    if (x < 0 || y < 0) {
        fieldWidget_->clearPreview();
        return;
    }
    highlightPreview(y, x);
}

void FieldSettingPage::highlightPreview(int row, int col) {
    int length = 4 - selectedShipType_;
    int dr = horizontal_ ? 0 : 1;
    int dc = horizontal_ ? 1 : 0;
    QVector<QPoint> cells;
    for (int i = 0; i < length; ++i) {
        int r = row + i * dr;
        int c = col + i * dc;
        if (r >= 0 && r < 10 && c >= 0 && c < 10)
            cells.append(QPoint(c, r));
    }
    bool can = canPlaceShip(row, col, length, horizontal_);
    QColor color = can ? QColor(255, 255, 0, 100) : QColor(255, 0, 0, 100);
    fieldWidget_->setPreview(cells, color);
}

bool FieldSettingPage::canPlaceShip(int row, int col, int length, bool horizontal) {
    int dr = horizontal ? 0 : 1;
    int dc = horizontal ? 1 : 0;
    for (int i = 0; i < length; ++i) {
        int r = row + i * dr;
        int c = col + i * dc;
        if (r >= 10 || c >= 10 || field_[r][c] != 0) return false;
        for (int ar = r - 1; ar <= r + 1; ++ar) {
            for (int ac = c - 1; ac <= c + 1; ++ac) {
                if (ar >= 0 && ar < 10 && ac >= 0 && ac < 10 && field_[ar][ac] == 1)
                    return false;
            }
        }
    }
    return true;
}

void FieldSettingPage::placeShipAt(int row, int col, int length, bool horizontal) {
    int dr = horizontal ? 0 : 1;
    int dc = horizontal ? 1 : 0;
    for (int i = 0; i < length; ++i) {
        int r = row + i * dr;
        int c = col + i * dc;
        field_[r][c] = 1;
    }
    updateFieldDisplay();
}

void FieldSettingPage::updateFieldDisplay() {
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            CellState state = (field_[y][x] == 1) ? CellState::SHIP : CellState::EMPTY;
            fieldWidget_->setCellState(x, y, state);
        }
    }
}

void FieldSettingPage::clearLocalField() {
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x)
            field_[y][x] = 0;
    placedShips_ = QVector<int>(4, 0);
    selectedShipType_ = -1;
    updateFieldDisplay();
}

void FieldSettingPage::checkReady() {
    if (placedShips_[0] >= 1 && placedShips_[1] >= 2 && placedShips_[2] >= 3 && placedShips_[3] >= 4) {
        readyButton_->setEnabled(true);
        statusLabel_->setText("Все корабли размещены. Нажмите 'Готов'");
    } else {
        readyButton_->setEnabled(false);
    }
}

void FieldSettingPage::onReadyClicked() {
    network_->sendCommand("ready");
    statusLabel_->setText("Ожидание готовности соперника...");
    readyButton_->setEnabled(false);
}

void FieldSettingPage::onGameStarted() {
    statusLabel_->setText("Игра начинается!");
    emit gameStarted();
}

void FieldSettingPage::onClearFieldClicked() {
    network_->sendCommand("clear");
    statusLabel_->setText("Очистка поля...");
    clearButton_->setEnabled(false);
    clearLocalField();
    checkReady();
}

void FieldSettingPage::onFieldCleared() {
    statusLabel_->setText("Поле очищено. Расставьте корабли заново.");
    clearButton_->setEnabled(true);
    readyButton_->setEnabled(false);
}
