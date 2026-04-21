#include "ui/login_page.hpp"
#include <QVBoxLayout>
#include <QLabel>

LoginPage::LoginPage(ClientNetwork* network, QWidget* parent) : QWidget(parent), network_(network) {
    setupUI();
    connect(network_, &ClientNetwork::connected, this, &LoginPage::onConnected);
    connect(network_, &ClientNetwork::errorOccurred, this, &LoginPage::onError);
    connect(network_, &ClientNetwork::loginResult, this, &LoginPage::onLoginResult);
    connect(network_, &ClientNetwork::signupResult, this, &LoginPage::onSignupResult);
}

LoginPage::~LoginPage() = default;

void LoginPage::onConnectClicked() {
    QString host = hostEdit_->text();
    quint16 port = portEdit_->text().toUShort();

    if (host.isEmpty()) {
        appendLog("Error: Host cannot be empty");
        return;
    }
    
    if (port == 0) {
        appendLog("Error: Invalid port");
        return;
    }

    appendLog(QString("Connecting to %1:%2...").arg(host).arg(port));
    connectButton_->setEnabled(false);
    network_->connectToServer(host, port);
}

void LoginPage::onSignupClicked() {
    QString login = loginEdit_->text();
    QString password = passwordEdit_->text();
    
    if (login.isEmpty()) {
        appendLog("Error: Login cannot be empty");
        return;
    }
    
    if (password.isEmpty()) {
        appendLog("Error: Password cannot be empty");
        return;
    }
    
    appendLog(QString("Registering user '%1'...").arg(login));
    network_->sendCommand(QString("signup %1 %2").arg(login).arg(password));
}

void LoginPage::onConnected() {
    appendLog("Connected to server!");
    connectButton_->setEnabled(false);
    connectButton_->setVisible(false);
    loginButton_->setEnabled(true);
    signupButton_->setEnabled(true);
}

void LoginPage::onError(const QString& error) {
    appendLog("Error: " + error);
    connectButton_->setEnabled(true);
}

void LoginPage::onLoginClicked() {
    QString login = loginEdit_->text();
    QString password = passwordEdit_->text();

    if (login.isEmpty()) {
        appendLog("Error: Login cannot be empty");
        return;
    }
    
    if (password.isEmpty()) {
        appendLog("Error: Password cannot be empty");
        return;
    }

    appendLog("Sending login...");
    network_->sendCommand(QString("login %1 %2").arg(login).arg(password));
}

void LoginPage::onLoginResult(bool success, const QString& message) {
    if (success) {
        appendLog("Login successful!");
        playButton_->setEnabled(true);
        loginButton_->setEnabled(false);
        signupButton_->setEnabled(false);
    } else {
        appendLog("Login failed: " + message);
    }
}

void LoginPage::onSignupResult(bool success, const QString& message) {
    if (success) {
        appendLog("Registration successful! You are now logged in.");
        playButton_->setEnabled(true);
        loginButton_->setEnabled(false);
        signupButton_->setEnabled(false);
    } else {
        appendLog("Registration failed: " + message);
    }
}

void LoginPage::onPlayClicked() {
    emit playButtonClicked();
}

void LoginPage::setupUI() {
    hostEdit_ = new QLineEdit(this);
    hostEdit_->setPlaceholderText("IP адрес сервера");
    hostEdit_->setText("127.0.0.1");
    
    portEdit_ = new QLineEdit(this);
    portEdit_->setPlaceholderText("Порт");
    portEdit_->setText("8080");
    
    loginEdit_ = new QLineEdit(this);
    loginEdit_->setPlaceholderText("Логин");
    
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setPlaceholderText("Пароль");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    
    logWidget_ = new QTextEdit(this);
    logWidget_->setReadOnly(true);
    logWidget_->setMaximumHeight(150);

    connectButton_ = new QPushButton("Подключиться к серверу", this);
    loginButton_ = new QPushButton("Войти в аккаунт", this);
    signupButton_ = new QPushButton("Зарегистрироваться", this);
    playButton_ = new QPushButton("Играть", this);
    
    loginButton_->setEnabled(false);
    signupButton_->setEnabled(false);
    playButton_->setEnabled(false);
    
    connect(connectButton_, &QPushButton::clicked, this, &LoginPage::onConnectClicked);
    connect(loginButton_, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(signupButton_, &QPushButton::clicked, this, &LoginPage::onSignupClicked);
    connect(playButton_, &QPushButton::clicked, this, &LoginPage::onPlayClicked);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout* connLayout = new QHBoxLayout();
    connLayout->addWidget(new QLabel("Хост:", this));
    connLayout->addWidget(hostEdit_);
    connLayout->addWidget(new QLabel("Порт", this));
    connLayout->addWidget(portEdit_);
    connLayout->addWidget(connectButton_);
    
    QHBoxLayout* authLayout = new QHBoxLayout();
    authLayout->addWidget(new QLabel("Логин:", this));
    authLayout->addWidget(loginEdit_);
    authLayout->addWidget(new QLabel("Пароль:", this));
    authLayout->addWidget(passwordEdit_);
    authLayout->addWidget(loginButton_);
    authLayout->addWidget(signupButton_);
    
    QHBoxLayout* playLayout = new QHBoxLayout();
    playLayout->addStretch();
    playLayout->addWidget(playButton_);
    playLayout->addStretch();
    
    mainLayout->addLayout(connLayout);
    mainLayout->addLayout(authLayout);
    mainLayout->addWidget(logWidget_);
    mainLayout->addLayout(playLayout);
    
    setLayout(mainLayout);
}

void LoginPage::appendLog(const QString& message) {
    logWidget_->append(message);
}
