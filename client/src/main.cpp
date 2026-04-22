#include <QApplication>
#include "ui/main_window.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setStyleSheet(R"(
        QWidget {
            background-color: #ebfcff;
        }

        QPushButton {
            background-color: #5c9ec9;
            color: #000000;
            padding: 8px;
            border-radius: 6px;
            border: none;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #1c6ea4;
        }

        QPushButton:disabled {
            background-color: #9ebccf;
            color: #7f8c8d;
            border: none;
        }

        QLineEdit {
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            padding: 6px;
            color: #000000;
        }
        QLineEdit:focus {
            border-color: #3498db;
            outline: none;
        }

        QTextEdit {
            background-color: #b1c0cf;
            color: #000000;
            font-family: monospace;
            border: 1px solid #8698aa;
            border-radius: 4px;
            padding: 4px;
        }

        QLabel {
            color: #000000;
            font-size: 13px;
        }

        QPushButton#readyButton {
            background-color: #2ecc71;
        }
        QPushButton#readyButton:hover {
            background-color: #27ae60;
        }
        QPushButton#readyButton:pressed {
            background-color: #1e8449;
        }
        QPushButton#readyButton:disabled {
            background-color: #a3e4d7;
            color: #000000;
        }
    )");

    MainWindow window;
    window.show();
    
    return app.exec();
}