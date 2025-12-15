#include <QApplication>
#include <QPushButton>
#include <QApplication>
#include "mainwindow.h"
#include <QSslSocket>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    //qDebug() << "Supports SSL?           " << QSslSocket::supportsSsl();
    //qDebug() << "Build wants OpenSSL:    " << QSslSocket::sslLibraryBuildVersionString();
    //qDebug() << "Runtime OpenSSL version:" << QSslSocket::sslLibraryVersionString();
    MainWindow w;
    w.setWindowTitle("数据结构可视化");
    w.show();
    return app.exec();
}