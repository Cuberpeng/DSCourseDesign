//
// Created by xiang on 25-9-28.
//
//
// Reworked by ChatGPT on 2025-10-16.
// 全新布局：QSplitter(左画布/右控制面板) + 下拉选择 + 分页表单 + 画布缩放工具栏
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSplitter>
#include <QScrollArea>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QToolBar>
#include <QAction>
#include <functional>
#include <QVector>

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
    // 顺序表
    void seqlistBuild();
    void seqlistInsert();
    void seqlistErase();
    void seqlistClear();

    // 链表
    void linklistBuild();
    void linklistInsert();
    void linklistErase();
    void linklistClear();

    // 栈
    void stackBuild();
    void stackPush();
    void stackPop();
    void stackClear();

    // 二叉树
    void btBuild();
    void btClear();
    void btPreorder();
    void btInorder();
    void btPostorder();
    void animateBTOrder(const int* order, int n, const QString& title);

    // 二叉搜索树
    void bstBuild();
    void bstFind();
    void bstErase();
    void bstClear();

    // 哈夫曼树
    void huffmanBuild();
    void huffmanClear();

    // 画布缩放
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onZoomReset();

private:
    //布局核心
    QSplitter* splitter{};
    Canvas* view{};                  // 左侧画布（可缩放/拖拽）
    QWidget* controlPanel{};         // 右侧控制面板
    QComboBox* moduleCombo{};        // 下拉选择模块
    QStackedWidget* moduleStack{};   // 每个模块一页
    QToolBar* canvasBar{};           // 顶部工具栏（缩放控制）

    //播放队列
    QTimer timer;
    QVector<std::function<void()>> steps;
    int stepIndex = 0;
    void playSteps();

    //绘制助手
    void drawSeqlist(const ds::Seqlist& sl);
    void drawLinklist(const ds::Linklist& ll);
    void drawStack(const ds::Stack& st);
    void drawBT(ds::BTNode* root, qreal x, qreal y, qreal distance, int highlightKey=-99999);

    //右侧控件
    // 顺序表
    QLineEdit* seqlistInput{};
    QLineEdit* seqlistValue{};
    QSpinBox* seqlistPosition{};

    // 链表
    QLineEdit* linklistInput{};
    QLineEdit* linklistValue{};
    QSpinBox* linklistPosition{};

    // 栈
    QLineEdit* stackInput{};
    QLineEdit* stackValue{};

    // 二叉树
    QLineEdit* btInput{};
    QSpinBox* btNull{};

    // BST
    QLineEdit* bstInput{};
    QLineEdit* bstValue{};

    // 哈夫曼树
    QLineEdit* huffmanInput{};

    //后端状态
    ds::Seqlist seq;
    ds::Linklist link;
    ds::Stack st;
    ds::BinaryTree bt;
    ds::BinarySearchTree bst;
    ds::Huffman huff;

    //工具
    QVector<int> parseIntList(const QString& text) const;

    // 右侧表单页构建
    QWidget* buildSeqlistPage();
    QWidget* buildLinklistPage();
    QWidget* buildStackPage();
    QWidget* buildBTPage();
    QWidget* buildBSTPage();
    QWidget* buildHuffmanPage();

    static QWidget* makeScrollPage(QWidget* content); // 放进 QScrollArea
};

#endif // MAINWINDOW_H
