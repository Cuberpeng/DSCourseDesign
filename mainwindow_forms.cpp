//
// Created by xiang on 25-11-6.
//

#include "mainwindow.h"
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>

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
    auto* v = new QVBoxLayout(root);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(10);

    // ===== 权值输入 =====
    auto* form = new QWidget;
    auto* f = new QFormLayout(form);
    f->setContentsMargins(6, 6, 6, 6);
    f->setSpacing(6);

    huffmanInput = new QLineEdit;
    huffmanInput->setPlaceholderText(QStringLiteral("例如：5 7 2 13 9（用空格或逗号分隔）"));
    f->addRow(QStringLiteral("权值序列"), huffmanInput);

    // ===== 构建 / 清空 按钮行 =====
    auto* row0 = new QWidget;
    auto* hb0 = new QHBoxLayout(row0);
    hb0->setContentsMargins(6, 6, 6, 6);
    hb0->setSpacing(8);

    auto* btnBuild = new QPushButton(QStringLiteral("构建哈夫曼树"));
    auto* btnClear = new QPushButton(QStringLiteral("清空"));

    btnBuild->setMinimumHeight(32);
    btnClear->setMinimumHeight(32);

    btnBuild->setStyleSheet(
        "QPushButton{background:#3b82f6;color:white;border-radius:6px;padding:6px 12px;}"
        "QPushButton:hover{background:#2563eb;}"
    );
    btnClear->setStyleSheet(
        "QPushButton{background:#e11d48;color:white;border-radius:6px;padding:6px 12px;}"
        "QPushButton:hover{background:#be123c;}"
    );

    hb0->addWidget(btnBuild, 1);
    hb0->addWidget(btnClear, 0);

    // ===== 新增：编码结果表格 =====
    auto* codeWidget = new QWidget;
    auto* codeLayout = new QVBoxLayout(codeWidget);
    codeLayout->setContentsMargins(6, 6, 6, 6);
    codeLayout->setSpacing(6);

    auto* codeHint = new QLabel(
        QStringLiteral("构建完成后，将自动列出每个原始权值对应的哈夫曼编码，"
                       "方便对照树形结构理解编码体系。"));
    codeHint->setWordWrap(true);
    codeHint->setStyleSheet(
        "QLabel{color:#64748b;font-size:11px;}"
    );
    codeLayout->addWidget(codeHint);

    // 表格本体
    huffmanCodeTable = new QTableWidget(0, 3, codeWidget);
    QStringList headers;
    headers << QStringLiteral("序号")
            << QStringLiteral("权值")
            << QStringLiteral("哈夫曼编码");
    huffmanCodeTable->setHorizontalHeaderLabels(headers);

    huffmanCodeTable->horizontalHeader()->setStretchLastSection(true);
    huffmanCodeTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    huffmanCodeTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    huffmanCodeTable->verticalHeader()->setVisible(false);

    huffmanCodeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    huffmanCodeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    huffmanCodeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    huffmanCodeTable->setAlternatingRowColors(true);
    huffmanCodeTable->setMinimumHeight(140);

    huffmanCodeTable->setStyleSheet(
        "QTableWidget{"
        " border:1px solid #e2e8f0;"
        " border-radius:6px;"
        " gridline-color:#e2e8f0;"
        " font-size:11px;"
        "}"
        "QHeaderView::section{"
        " background:#f1f5f9;"
        " border:0px;"
        " padding:4px 6px;"
        " color:#475569;"
        " font-weight:600;"
        " font-size:11px;"
        "}"
    );

    codeLayout->addWidget(huffmanCodeTable, 1);

    // ===== 组装到整体布局 =====
    v->addWidget(wrapGroup(QStringLiteral("哈夫曼树"), form));
    v->addWidget(wrapGroup(QStringLiteral("构建 / 清空"), row0));
    v->addWidget(wrapGroup(QStringLiteral("编码结果（原始权值 → 哈夫曼码）"), codeWidget));
    v->addStretch(1);

    // 信号连接
    connect(btnBuild, &QPushButton::clicked, this, &MainWindow::huffmanBuild);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::huffmanClear);

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
    auto* h = new QHBoxLayout(root);
    h->setSpacing(12);

    //
    // 左侧：DSL 脚本编辑与执行
    //
    auto* dslGroup = new QGroupBox(QStringLiteral("DSL 脚本"));
    auto* dslLayout = new QVBoxLayout(dslGroup);
    dslLayout->setSpacing(8);

    dslEdit = new QTextEdit;
    dslEdit->setPlaceholderText(
        "指令格式可点击下侧“DSL使用说明”查看\n"
        "每行只能填写一条语句"
        "只能输入同一种数据结构中的指令"
    );
    dslEdit->setFixedHeight(100);
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

    auto* btnRun  = new QPushButton(QStringLiteral("执行脚本"));
    btnRun->setStyleSheet("QPushButton{background:#22c55e;color:white;}");
    auto* btnHelp = new QPushButton(QStringLiteral("DSL 使用说明"));   // 打开说明页
    btnHelp->setStyleSheet("QPushButton{background:#3b82f6;color:white;}");

    auto* dslBtns = new QHBoxLayout;
    dslBtns->addWidget(btnRun);
    dslBtns->addWidget(btnHelp);

    dslLayout->addWidget(new QLabel(QStringLiteral("DSL 脚本：")));
    dslLayout->addWidget(dslEdit);
    dslLayout->addLayout(dslBtns);
    dslLayout->addStretch(1);

    //
    // 右侧：自然语言输入
    //
    auto* nliGroup = new QGroupBox(QStringLiteral("自然语言指令"));
    auto* nliLayout = new QVBoxLayout(nliGroup);
    nliLayout->setSpacing(8);

    nliEdit = new QTextEdit;
    nliEdit->setPlaceholderText(QStringLiteral("自然语言指令，如：创建一个包含数据元素[5,3,7,2,4]的二叉搜索树，注意只能对同一种数据结构进行操作"));
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

    auto* btnNLI = new QPushButton(QStringLiteral("理解并执行"));
    btnNLI->setStyleSheet("QPushButton{background:#8b5cf6;color:white;}");

    nliLayout->addWidget(new QLabel(QStringLiteral("自然语言（将自动转为 DSL 后执行）：")));
    nliLayout->addWidget(nliEdit);
    nliLayout->addWidget(btnNLI);
    nliLayout->addStretch(1);

    // 总体左右排布
    h->addWidget(dslGroup, 1);
    h->addWidget(nliGroup, 1);

    // 事件连接（逻辑保持不变）
    connect(btnRun,  &QPushButton::clicked, this, &MainWindow::runDSL);
    connect(btnHelp, &QPushButton::clicked, this, &MainWindow::insertDSLExample);
    connect(btnNLI,  &QPushButton::clicked, this, &MainWindow::runNLI);

    return root;
}
