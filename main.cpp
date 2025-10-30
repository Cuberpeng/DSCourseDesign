#include <QApplication>
#include <QPushButton>

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.setWindowTitle("数据结构可视化");
    w.show();
    return app.exec();
}