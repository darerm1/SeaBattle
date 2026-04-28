#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <memory>

class ClientNetwork;
class LoginPage;
class FieldSettingPage;
class GamePage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void switchToLoginPage();
    void switchToFieldSettingPage();
    void switchToGamePage();

private:
    void setupUI();
    
    std::unique_ptr<ClientNetwork> network_;
    QStackedWidget* stackedWidget_;
    
    LoginPage* loginPage_;
    FieldSettingPage* fieldSettingPage_;
    GamePage* gamePage_;
};
