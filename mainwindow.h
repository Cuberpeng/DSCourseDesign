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
#include "linklist.h"
#include "stack.h"
#include "binarytree.h"
#include "binarysearchtree.h"
#include "huffman.h"

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

    //链表
    void linklistBuild();
    void linklistInsert();
    void linklistErase();
    void linklistClear();

    //栈
    void stackBuild();
    void stackPush();
    void stackPop();
    void stackClear();

    //二叉树
    void btBuild();
    void btClear();

    //二叉搜索树
    void bstBuild();
    void bstFind();
    void bstErase();
    void bstClear();

    //哈夫曼树
    void huffmanBuild();
    void huffmanClear();

private:
    Canvas* view{};
    QTimer timer;
    QVector<std::function<void()>> steps;
    int stepIndex = 0;
    void playSteps();


    // 布局绘制助手
    void drawSeqlist(const ds::Seqlist& sl);
    void drawLinklist(const ds::Linklist& ll);
    void drawStack(const ds::Stack& st);
    void drawBT(ds::BTNode* root, qreal x, qreal y, qreal distance, int highlightKey=-99999);
    //int subtreeWidth(ds::BTNode* root);
    //void drawHuff(ds::HNode* root, qreal x, qreal y, qreal xspan);


    // —— 交互面板 ——
    QDockWidget* dock{};
    QTabWidget* tabs{};


    // 顺序表
    QLineEdit* seqlistInput{}; // 例如：1 3 5 7
    QLineEdit* seqlistValue{}; // 值
    QSpinBox* seqlistPosition{}; // 位置

    //链表
    QLineEdit* linklistInput{};
    QLineEdit* linklistValue{};
    QSpinBox* linklistPosition{};

    //栈
    QLineEdit* stackInput{};
    QLineEdit* stackValue{};

    //二叉树
    QLineEdit* btInput{}; // 例如：15 6 23 4 7 17 71 5 -1 -1 50
    QSpinBox* btNull{}; // 缺失哨兵，默认 -1

    // 二叉搜索树
    QLineEdit* bstInput{}; // 初始批量插入
    QLineEdit* bstValue{}; // 单值操作

    //哈夫曼树
    QLineEdit* huffmanInput{};


    // 后端持久状态
    ds::Seqlist seq;
    ds::Linklist link;
    ds::Stack st;
    ds::BinaryTree bt;
    ds::BinarySearchTree bst;
    ds::Huffman huff;

    // 工具
    QVector<int> parseIntList(const QString& text) const;

};

#endif //MAINWINDOW_H
