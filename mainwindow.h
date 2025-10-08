//
// Created by xiang on 25-9-28.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QDockWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <functional>

#include "canvas.h"
#include "seqlist.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
    explicit MainWindow(QWidget* parent=nullptr);

private slots:

    //顺序表
    void seqlistBuild();
    void seqlistInsert();
    void seqlistErase();
    void seqlistClear();

private:
    Canvas* view{};
    QTimer timer;
    QVector<std::function<void()>> steps;
    int stepIndex = 0;
    void playSteps();


    // 布局绘制助手
    void drawSeq(const ds::Seqlist& sl);
    //void drawLinked(const ds::LinkedList& lst);
    //void drawStack(const ds::Stack& st);
    //void drawBT(ds::BTNode* root, qreal x, qreal y, qreal xspan, int depth, int highlightKey=-99999);
    //int subtreeWidth(ds::BTNode* root);
    //void drawHuff(ds::HNode* root, qreal x, qreal y, qreal xspan);


    // —— 交互面板 ——
    QDockWidget* dock{};
    QTabWidget* tabs{};


    // 顺序表
    QLineEdit* seqBulkEdit{}; // 例如：1 3 5 7
    QLineEdit* seqValEdit{}; // 值
    QSpinBox* seqPosSpin{}; // 位置


    // 后端持久状态
    ds::Seqlist seq;

    // 工具
    QVector<int> parseIntList(const QString& text) const;

};

#endif //MAINWINDOW_H
