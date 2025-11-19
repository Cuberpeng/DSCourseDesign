//
// Created by xiang on 25-11-6.
//
//
// mainwindow_pages.cpp
// 右侧彩色表单页与控件（仅负责构建 UI）
//
#include "mainwindow.h"
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

static QWidget* wrapGroup(const QString& title, QWidget* inner) {
    auto* box = new QGroupBox(title);
    auto* v = new QVBoxLayout(box);
    v->setContentsMargins(12,10,12,12);
    v->addWidget(inner);
    return box;
}

QWidget* MainWindow::buildSeqlistPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    auto* form = new QWidget; auto* f = new QFormLayout(form);
    seqlistInput = new QLineEdit; seqlistInput->setPlaceholderText("例如: 1 3 5 7");
    f->addRow("初始序列", seqlistInput);

    auto* row0 = new QWidget; auto* hb0 = new QHBoxLayout(row0);
    auto* btnRebuild = new QPushButton("建立"); btnRebuild->setProperty("class","primary"); btnRebuild->setObjectName("btnSLBuild");
    auto* btnClear   = new QPushButton("清空"); btnClear->setProperty("class","warn");
    btnRebuild->setStyleSheet("QPushButton{background:#22c55e;}"); // 绿
    btnClear->setStyleSheet("QPushButton{background:#ef4444;}");   // 红
    hb0->addWidget(btnRebuild); hb0->addWidget(btnClear);

    auto* row1 = new QWidget; auto* hb1 = new QHBoxLayout(row1);
    seqlistValue = new QLineEdit; seqlistValue->setPlaceholderText("值");
    seqlistPosition = new QSpinBox; seqlistPosition->setRange(0, 1000000);
    auto* btnInsert = new QPushButton("插入"); btnInsert->setStyleSheet("QPushButton{background:#3b82f6;}");
    auto* btnErase  = new QPushButton("删除"); btnErase->setStyleSheet("QPushButton{background:#f59e0b;}");
    hb1->addWidget(new QLabel("值:")); hb1->addWidget(seqlistValue);
    hb1->addWidget(new QLabel("位置:")); hb1->addWidget(seqlistPosition);
    hb1->addWidget(btnInsert); hb1->addWidget(btnErase);

    v->addWidget(wrapGroup("顺序表建立", form));
    v->addWidget(wrapGroup("顺序表操作", row0));
    v->addWidget(wrapGroup("插入/删除", row1));
    v->addStretch(1);

    connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::seqlistBuild);
    connect(btnClear, &QPushButton::clicked,this,&MainWindow::seqlistClear);
    connect(btnInsert,&QPushButton::clicked,this,&MainWindow::seqlistInsert);
    connect(btnErase, &QPushButton::clicked,this,&MainWindow::seqlistErase);
    return root;
}

QWidget* MainWindow::buildLinklistPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    auto* form = new QWidget; auto* f = new QFormLayout(form);
    linklistInput = new QLineEdit; linklistInput->setPlaceholderText("例如: 1 3 5 7");
    f->addRow("初始序列", linklistInput);

    auto* row0 = new QWidget; auto* hb0 = new QHBoxLayout(row0);
    auto* btnRebuild = new QPushButton("建立"); btnRebuild->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnClear   = new QPushButton("清空"); btnClear->setStyleSheet("QPushButton{background:#ef4444;color:white;}");
    hb0->addWidget(btnRebuild); hb0->addWidget(btnClear);

    auto* row1 = new QWidget; auto* hb1 = new QHBoxLayout(row1);
    linklistValue = new QLineEdit; linklistValue->setPlaceholderText("值");
    linklistPosition = new QSpinBox; linklistPosition->setRange(0,1000000);
    auto* btnIns = new QPushButton("插入"); btnIns->setStyleSheet("QPushButton{background:#3b82f6;color:white;}");
    auto* btnDel = new QPushButton("删除"); btnDel->setStyleSheet("QPushButton{background:#f59e0b;color:white;}");
    hb1->addWidget(new QLabel("值:")); hb1->addWidget(linklistValue);
    hb1->addWidget(new QLabel("位置:")); hb1->addWidget(linklistPosition);
    hb1->addWidget(btnIns); hb1->addWidget(btnDel);

    v->addWidget(wrapGroup("单链表建立", form));
    v->addWidget(wrapGroup("单链表操作", row0));
    v->addWidget(wrapGroup("插入/删除", row1));
    v->addStretch(1);

    connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::linklistBuild);
    connect(btnClear, &QPushButton::clicked,this,&MainWindow::linklistClear);
    connect(btnIns,   &QPushButton::clicked,this,&MainWindow::linklistInsert);
    connect(btnDel,   &QPushButton::clicked,this,&MainWindow::linklistErase);
    return root;
}

QWidget* MainWindow::buildStackPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    auto* form = new QWidget; auto* f = new QFormLayout(form);
    stackInput = new QLineEdit; stackInput->setPlaceholderText("例如: 1 3 5 7");
    f->addRow("初始序列", stackInput);

    auto* row0 = new QWidget; auto* hb0 = new QHBoxLayout(row0);
    auto* btnRebuild = new QPushButton("建立"); btnRebuild->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnClear   = new QPushButton("清空"); btnClear->setStyleSheet("QPushButton{background:#ef4444;color:white;}");
    hb0->addWidget(btnRebuild); hb0->addWidget(btnClear);

    auto* row1 = new QWidget; auto* hb1 = new QHBoxLayout(row1);
    stackValue = new QLineEdit; stackValue->setPlaceholderText("值");
    auto* btnPush = new QPushButton("入栈"); btnPush->setStyleSheet("QPushButton{background:#3b82f6;color:white;}");
    auto* btnPop  = new QPushButton("出栈"); btnPop ->setStyleSheet("QPushButton{background:#f59e0b;color:white;}");
    hb1->addWidget(new QLabel("值:")); hb1->addWidget(stackValue);
    hb1->addWidget(btnPush); hb1->addWidget(btnPop);

    v->addWidget(wrapGroup("顺序栈建立", form));
    v->addWidget(wrapGroup("顺序栈操作", row0));
    v->addWidget(wrapGroup("入栈/出栈", row1));
    v->addStretch(1);

    connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::stackBuild);
    connect(btnClear, &QPushButton::clicked,this,&MainWindow::stackClear);
    connect(btnPush,  &QPushButton::clicked,this,&MainWindow::stackPush);
    connect(btnPop,   &QPushButton::clicked,this,&MainWindow::stackPop);
    return root;
}

QWidget* MainWindow::buildBTPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    auto* form = new QWidget; auto* f = new QFormLayout(form);
    btInput = new QLineEdit; btInput->setPlaceholderText("层序，空位用哨兵，如: 15 6 23 4 7 17 71 5 -1 -1 50");
    btNull  = new QSpinBox; btNull->setRange(-1000000,1000000); btNull->setValue(-1);
    f->addRow("层序数组", btInput);
    f->addRow("空位哨兵", btNull);

    auto* row0 = new QWidget; auto* hb0 = new QHBoxLayout(row0);
    auto* btnBuild = new QPushButton("建立"); btnBuild->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnClear = new QPushButton("清空"); btnClear->setStyleSheet("QPushButton{background:#ef4444;color:white;}");
    hb0->addWidget(btnBuild); hb0->addWidget(btnClear);

    auto* row1 = new QWidget; auto* hb1 = new QHBoxLayout(row1);
    auto* btnPre  = new QPushButton("先序遍历"); btnPre ->setStyleSheet("QPushButton{background:#3b82f6;color:white;}");
    auto* btnIn   = new QPushButton("中序遍历"); btnIn  ->setStyleSheet("QPushButton{background:#6366f1;color:white;}");
    auto* btnPost = new QPushButton("后序遍历"); btnPost->setStyleSheet("QPushButton{background:#a855f7;color:white;}");
    auto* btnLevel = new QPushButton("层序遍历"); btnLevel->setStyleSheet("QPushButton{background:#10b981;color:white;}"); // 新增按钮
    hb1->addWidget(btnPre); hb1->addWidget(btnIn); hb1->addWidget(btnPost); hb1->addWidget(btnLevel);

    v->addWidget(wrapGroup("二叉树建立", form));
    v->addWidget(wrapGroup("二叉树操作", row0));
    v->addWidget(wrapGroup("遍历演示", row1));
    v->addStretch(1);

    connect(btnBuild,&QPushButton::clicked,this,&MainWindow::btBuild);
    connect(btnClear,&QPushButton::clicked,this,&MainWindow::btClear);
    connect(btnPre,  &QPushButton::clicked,this,&MainWindow::btPreorder);
    connect(btnIn,   &QPushButton::clicked,this,&MainWindow::btInorder);
    connect(btnPost, &QPushButton::clicked,this,&MainWindow::btPostorder);
    connect(btnLevel, &QPushButton::clicked,this,&MainWindow::btLevelorder); // 连接新按钮
    return root;
}

QWidget* MainWindow::buildBSTPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    auto* form = new QWidget; auto* f = new QFormLayout(form);
    bstInput = new QLineEdit; bstInput->setPlaceholderText("例如: 15 6 23 4 7 17 71");
    f->addRow("初始序列", bstInput);

    auto* row0 = new QWidget; auto* hb0 = new QHBoxLayout(row0);
    auto* btnBuild = new QPushButton("建立"); btnBuild->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnClear = new QPushButton("清空"); btnClear->setStyleSheet("QPushButton{background:#ef4444;color:white;}");
    hb0->addWidget(btnBuild); hb0->addWidget(btnClear);

    auto* row1 = new QWidget; auto* hb1 = new QHBoxLayout(row1);
    bstValue = new QLineEdit; bstValue->setPlaceholderText("键值");
    auto* btnFind = new QPushButton("查找"); btnFind->setStyleSheet("QPushButton{background:#3b82f6;color:white;}");
    auto* btnInsert = new QPushButton("插入"); btnInsert->setStyleSheet("QPushButton{background:#10b981;color:white;}"); // 新增插入按钮
    auto* btnDel  = new QPushButton("删除"); btnDel ->setStyleSheet("QPushButton{background:#f59e0b;color:white;}");
    hb1->addWidget(new QLabel("值:")); hb1->addWidget(bstValue);
    hb1->addWidget(btnFind); hb1->addWidget(btnInsert); hb1->addWidget(btnDel); // 添加插入按钮

    v->addWidget(wrapGroup("BST 建立", form));
    v->addWidget(wrapGroup("BST 操作", row0));
    v->addWidget(wrapGroup("查找/插入/删除", row1)); // 修改组标题
    v->addStretch(1);

    connect(btnBuild,&QPushButton::clicked,this,&MainWindow::bstBuild);
    connect(btnClear,&QPushButton::clicked,this,&MainWindow::bstClear);
    connect(btnFind, &QPushButton::clicked,this,&MainWindow::bstFind);
    connect(btnInsert, &QPushButton::clicked,this,&MainWindow::bstInsert); // 连接插入按钮
    connect(btnDel,  &QPushButton::clicked,this,&MainWindow::bstErase);
    return root;
}

QWidget* MainWindow::buildHuffmanPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    auto* form = new QWidget; auto* f = new QFormLayout(form);
    huffmanInput = new QLineEdit; huffmanInput->setPlaceholderText("权值序列，如: 5 7 2 9 3");
    f->addRow("权值序列", huffmanInput);

    auto* row0 = new QWidget; auto* hb0 = new QHBoxLayout(row0);
    auto* btnBuild = new QPushButton("建立"); btnBuild->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnClear = new QPushButton("清空"); btnClear->setStyleSheet("QPushButton{background:#ef4444;color:white;}");
    hb0->addWidget(btnBuild); hb0->addWidget(btnClear);

    v->addWidget(wrapGroup("哈夫曼树", form));
    v->addWidget(wrapGroup("构建/清空", row0));
    v->addStretch(1);

    connect(btnBuild,&QPushButton::clicked,this,&MainWindow::huffmanBuild);
    connect(btnClear,&QPushButton::clicked,this,&MainWindow::huffmanClear);
    return root;
}

QWidget* MainWindow::buildAVLPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    auto* form = new QWidget; auto* f = new QFormLayout(form);
    avlInput = new QLineEdit; avlInput->setPlaceholderText("例如: 15 6 23 4 7 17 71");
    f->addRow("初始序列", avlInput);

    auto* row0 = new QWidget; auto* hb0 = new QHBoxLayout(row0);
    auto* btnBuild = new QPushButton("建立"); btnBuild->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnClear = new QPushButton("清空"); btnClear->setStyleSheet("QPushButton{background:#ef4444;color:white;}");
    hb0->addWidget(btnBuild); hb0->addWidget(btnClear);

    auto* row1 = new QWidget; auto* hb1 = new QHBoxLayout(row1);
    avlValue = new QLineEdit; avlValue->setPlaceholderText("键值");
    auto* btnInsert = new QPushButton("插入"); btnInsert->setStyleSheet("QPushButton{background:#3b82f6;color:white;}");
    hb1->addWidget(new QLabel("值:")); hb1->addWidget(avlValue);
    hb1->addWidget(btnInsert);

    v->addWidget(wrapGroup("AVL树建立", form));
    v->addWidget(wrapGroup("AVL树操作", row0));
    v->addWidget(wrapGroup("插入", row1));
    v->addStretch(1);

    connect(btnBuild,&QPushButton::clicked,this,&MainWindow::avlBuild);
    connect(btnClear,&QPushButton::clicked,this,&MainWindow::avlClear);
    connect(btnInsert,&QPushButton::clicked,this,&MainWindow::avlInsert);
    return root;
}

QWidget* MainWindow::buildDSLPage() {
    auto* root = new QWidget;
    auto* v = new QVBoxLayout(root); v->setSpacing(10);

    // 上半：DSL 脚本编辑与执行
    dslEdit = new QTextEdit;
    dslEdit->setPlaceholderText(
        "在此编写 DSL 脚本：一行一条命令，# 开头为注释，大小写不敏感。\n"
        "例：\n"
        "  bst 15 6 23 4 7 17 71\n"
        "  bst.find 7\n"
        "  bt 15 6 23 4 -1 -1 7  null=-1\n"
        "（更多指令：点击右侧“DSL 使用说明”查看）"
    );
    dslEdit->setFixedHeight(120);
    dslEdit->setStyleSheet(
        "QTextEdit {"
        "   border: 2px solid #e2e8f0;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "   background: white;"
        "   font-size: 14px;"
        "}"
        "QTextEdit:focus {"
        "   border-color: #3b82f6;"
        "}"
    );

    auto* hb0 = new QHBoxLayout;
    auto* btnRun  = new QPushButton("执行脚本");
    btnRun->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnHelp = new QPushButton("DSL 使用说明");   // ← 新按钮
    btnHelp->setStyleSheet("QPushButton{background:#3b82f6;color:white;}");
    hb0->addWidget(btnRun);
    hb0->addWidget(btnHelp);

    // 下半：自然语言输入（保持原逻辑）
    nliEdit = new QTextEdit;
    nliEdit->setPlaceholderText("自然语言指令，如：创建一个包含数据元素[5,3,7,2,4]的二叉搜索树（支持多行）");
    nliEdit->setMaximumHeight(100);
    nliEdit->setStyleSheet(
        "QTextEdit {"
        "   border: 2px solid #e2e8f0;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "   background: white;"
        "   font-size: 14px;"
        "}"
        "QTextEdit:focus {"
        "   border-color: #3b82f6;"
        "}"
    );
    auto* btnNLI = new QPushButton("理解并执行");
    btnNLI->setStyleSheet("QPushButton{background:#8b5cf6;color:white;}");

    v->addWidget(new QLabel("DSL 脚本："));
    v->addWidget(dslEdit);
    v->addLayout(hb0);
    v->addWidget(new QLabel("自然语言（将自动转为 DSL 后执行）："));
    v->addWidget(nliEdit);
    v->addWidget(btnNLI);
    v->addStretch(1);

    // 事件连接
    connect(btnRun,  &QPushButton::clicked, this, &MainWindow::runDSL);
    connect(btnHelp, &QPushButton::clicked, this, &MainWindow::insertDSLExample); // ← 打开说明页
    connect(btnNLI,  &QPushButton::clicked, this, &MainWindow::runNLI);

    return root;
}