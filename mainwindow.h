//
// Created by xiang on 25-9-28.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSplitter>
#include <QScrollArea>
#include <QComboBox>
#include <QStackedWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QMenuBar>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QVector>
#include <functional>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QTableWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QSlider>

#include "canvas.h"
#include "seqlist.h"
#include "linklist.h"
#include "stack.h"
#include "binarytree.h"
#include "binarysearchtree.h"
#include "huffman.h"
#include "avl.h"
#include "llmclient.h"

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
    void btLevelorder();

    // 二叉搜索树
    void bstBuild();
    void bstFind();
    void bstInsert();
    void bstErase();
    void bstClear();

    // 哈夫曼树
    void huffmanBuild();
    void huffmanClear();

    // AVL树
    void avlBuild();
    void avlInsert();
    void avlClear();

    // 画布缩放
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onZoomReset();

    // 动画播放控制
    void onAnimPlay();
    //void onAnimPause();
    //void onAnimStep();
    void onAnimReplay();

    // 模块切换时同步画布
    void onModuleChanged(int index);

    //文件保存
    void saveDoc();
    void openDoc();

    //DSL
    void insertDSLExample();
    void runDSL();
    void runLLM();

    // 处理大模型结果
    void onLlmDslReady(const QString& dslText);
    void onLlmError(const QString& message);

    //辅助函数
    QVector<int> dumpBTLevel(ds::BTNode* root, int nullSentinel) const;
    void dumpPreorder(ds::BTNode* r, QVector<int>& out) const;
    void collectLeafWeights(ds::BTNode* r, QVector<int>& out) const;


private:
    ds::Seqlist seq;
    ds::Linklist link;
    ds::Stack st;
    ds::BinaryTree bt;
    ds::BinarySearchTree bst;
    ds::Huffman huff;
    ds::AVL avl;
    // 布局核心
    QSplitter* splitter{};
    Canvas* view{};                  // 左侧画布
    QWidget* controlPanel{};         // 右侧控制面板
    QComboBox* moduleCombo{};        // 模块选择
    QStackedWidget* moduleStack{};   // 每个模块一页
    QToolBar* canvasBar{};           // 顶部工具栏（缩放）

    // 动画控制相关
    QAction* actAnimPlayToggle{};    // 播放 / 暂停 合并按钮
    QAction* actAnimStep{};          // 单步
    QAction* actAnimReplay{};        // 重播
    QSlider* animSpeedSlider{};      // 速度调节滑块

    // 播放队列
    QTimer timer;
    QVector<std::function<void()>> steps;
    int stepIndex = 0;
    void playSteps();
    void updateAnimUiState();        // 根据当前状态刷新按钮
    void onAnimSpeedChanged(int value); // 速度滑块回调

    // 绘制助手
    void drawSeqlist(const ds::Seqlist& sl);
    void drawLinklist(const ds::Linklist& ll);
    void drawStack(const ds::Stack& st);
    void drawBT(ds::BTNode* root, qreal x, qreal y, qreal distance, int highlightKey=-99999);
    // 追加一次 AVL 插入动画步骤（不会 stop timer / clear steps）
    void drawAVL(int value, const QVector<int>& restoreKeys, int idx, int total);

    // 右侧控件
    // 顺序表
    QLineEdit* seqlistInput{}; QLineEdit* seqlistValue{}; QSpinBox* seqlistPosition{};
    // 链表
    QLineEdit* linklistInput{}; QLineEdit* linklistValue{}; QSpinBox* linklistPosition{};
    // 栈
    QLineEdit* stackInput{}; QLineEdit* stackValue{};
    // 二叉树
    QLineEdit* btInput{}; QSpinBox* btNull{};
    // BST
    QLineEdit* bstInput{}; QLineEdit* bstValue{};
    // 哈夫曼
    QLineEdit* huffmanInput{}; QTableWidget* huffmanCodeTable{};
    // AVL树
    QLineEdit* avlInput{}; QLineEdit* avlValue{};


    // 文件保存相关
    enum class DocKind { None, SeqList, LinkedList, Stack, BinaryTree, BST, Huffman, AVL };
    DocKind currentKind_ = DocKind::None;
    int btLastNullSentinel_ = -1;
    QVector<int> huffLastWeights_;

    // ★ 新增：树类细粒度动画（QTimeLine）统一用的时长，受速度滑块控制
    int animTimelineDurationMs_ = 700;

    // DSL/NLI 相关
    QTextEdit* dslEdit{};
    QTextEdit* llmEdit{};

    //信息显示栏
    QTextEdit* messageBar{};

    //大模型客户端
    LLMClient* llmClient{};

    // NLI 调用中的提示弹窗
    QDialog* llmProgressDialog{};


    // 工具
    QVector<int> parseIntList(const QString& text) const;
    void showMessage(const QString& message);
    void clearMessages();

    // 右侧表单页构建
    QWidget* buildSeqlistPage();
    QWidget* buildLinklistPage();
    QWidget* buildStackPage();
    QWidget* buildBTPage();
    QWidget* buildBSTPage();
    QWidget* buildHuffmanPage();
    QWidget* buildAVLPage();
    QWidget* buildDSLPage();
    static QWidget* makeScrollPage(QWidget* content); // 放进 QScrollArea
};

#endif // MAINWINDOW_H