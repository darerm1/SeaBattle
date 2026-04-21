#include "ui/main_window.hpp"
#include "ui/login_page.hpp"
#include "ui/field_setting_page.hpp"
#include "ui/game_page.hpp"
#include "network/client_network.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), network_(std::make_unique<ClientNetwork>(this)) {
    setupUI();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    stackedWidget_ = new QStackedWidget(this);
    
    loginPage_ = new LoginPage(network_.get(), this);
    fieldSettingPage_ = new FieldSettingPage(network_.get(), this);
    gamePage_ = new GamePage(network_.get(), this);
    
    stackedWidget_->addWidget(loginPage_);
    stackedWidget_->addWidget(fieldSettingPage_);
    stackedWidget_->addWidget(gamePage_);
    
    setCentralWidget(stackedWidget_);
    
    connect(loginPage_, &LoginPage::playButtonClicked, this, &MainWindow::switchToFieldSettingPage);
    connect(fieldSettingPage_, &FieldSettingPage::gameStarted, this, &MainWindow::switchToGamePage);
    
    switchToLoginPage();
}

void MainWindow::switchToLoginPage() {
    stackedWidget_->setCurrentWidget(loginPage_);
}

void MainWindow::switchToFieldSettingPage() {
    stackedWidget_->setCurrentWidget(fieldSettingPage_);
}

void MainWindow::switchToGamePage() {
    stackedWidget_->setCurrentWidget(gamePage_);
}
