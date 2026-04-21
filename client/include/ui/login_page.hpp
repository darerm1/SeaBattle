#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QPushButton>
#include "network/client_network.hpp"

class LoginPage : public QWidget {
    Q_OBJECT

public:
    LoginPage(ClientNetwork* network, QWidget* parent = nullptr);
    ~LoginPage();

signals:
    void loginSuccessful();
    void playButtonClicked();
    
public slots:
    void onConnectClicked();
    void onLoginClicked();
    void onSignupClicked();
    void onConnected();
    void onLoginResult(bool success, const QString& message);
    void onSignupResult(bool success, const QString& message);
    void onError(const QString& error);
    void onPlayClicked();

private:
    void appendLog(const QString& message);
    void setupUI();

    ClientNetwork* network_;
    QLineEdit* hostEdit_;
    QLineEdit* portEdit_;
    QLineEdit* loginEdit_;
    QLineEdit* passwordEdit_;
    QPushButton* connectButton_;
    QPushButton* loginButton_;
    QPushButton* signupButton_;
    QTextEdit* logWidget_;
    QPushButton* playButton_;
};
