//
// Created by xiang on 25-9-30.
//

#include "mainwindow.h"
#include <QStatusBar>
#include <QApplication>
#include <QPushButton>
#include <QStyle>
#include <QIcon>
#include <climits>
#include <QHash>
#include <QStringList>
#include <cmath>
#include <memory>

//构造 UI
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(1200, 740);
    statusBar()->showMessage("数据结构可视化");

    // 顶部工具栏
    canvasBar = addToolBar("画布");
    QAction* actZoomIn = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowUp),   "放大");
    QAction* actZoomOut = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowDown), "缩小");
    QAction* actFit = canvasBar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "适配");
    QAction* actReset = canvasBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), "重置");
    connect(actZoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(actZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(actFit, &QAction::triggered, this, &MainWindow::onZoomFit);
    connect(actReset, &QAction::triggered, this, &MainWindow::onZoomReset);

    // 中心：左右分栏
    splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // 左：画布（支持 Ctrl+滚轮缩放、空格/中键拖拽）
    view = new Canvas(splitter);
    splitter->addWidget(view);

    // 右：控制面板
    controlPanel = new QWidget(splitter);
    splitter->addWidget(controlPanel);
    splitter->setStretchFactor(0, 3); // 左 3
    splitter->setStretchFactor(1, 2); // 右 2

    auto* v = new QVBoxLayout(controlPanel);
    v->setContentsMargins(10, 8, 10, 8);
    v->setSpacing(8);

    moduleCombo = new QComboBox(controlPanel);
    moduleCombo->addItem("顺序表");
    moduleCombo->addItem("链表");
    moduleCombo->addItem("栈");
    moduleCombo->addItem("二叉树");
    moduleCombo->addItem("二叉搜索树");
    moduleCombo->addItem("哈夫曼树");
    v->addWidget(new QLabel("模块选择：", controlPanel));
    v->addWidget(moduleCombo);

    moduleStack = new QStackedWidget(controlPanel);
    v->addWidget(moduleStack, 1);

    // 构建各页
    moduleStack->addWidget(makeScrollPage(buildSeqlistPage()));
    moduleStack->addWidget(makeScrollPage(buildLinklistPage()));
    moduleStack->addWidget(makeScrollPage(buildStackPage()));
    moduleStack->addWidget(makeScrollPage(buildBTPage()));
    moduleStack->addWidget(makeScrollPage(buildBSTPage()));
    moduleStack->addWidget(makeScrollPage(buildHuffmanPage()));

    connect(moduleCombo, qOverload<int>(&QComboBox::currentIndexChanged), moduleStack, &QStackedWidget::setCurrentIndex);

    // 动画播放定时器
    connect(&timer, &QTimer::timeout, this, &MainWindow::playSteps);
    timer.setInterval(500); // 每步 0.5s

    // 初始画布标题
    view->setTitle("请选择右侧模块并操作");
}

// Scroll 包装
QWidget* MainWindow::makeScrollPage(QWidget* content) {
    auto* sa = new QScrollArea;
    sa->setWidget(content);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    return sa;
}

// ========== 各页 ==========
QWidget* MainWindow::buildSeqlistPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* f = new QFormLayout;
    v->addLayout(f);

    seqlistInput = new QLineEdit;
    seqlistInput->setPlaceholderText("例如: 1 3 5 7");
    f->addRow("初始序列", seqlistInput);

    // 操作行
    auto* hb0 = new QHBoxLayout;
    v->addLayout(hb0);
    auto* btnRebuild = new QPushButton("建立");
    auto* btnClear = new QPushButton("清空");
    hb0->addWidget(btnRebuild);
    hb0->addWidget(btnClear);

    // 插入和删除
    auto* hb1 = new QHBoxLayout;
    v->addLayout(hb1);
    seqlistValue = new QLineEdit; seqlistValue->setPlaceholderText("值");
    seqlistPosition = new QSpinBox; seqlistPosition->setRange(0, 1000000);
    auto* btnInsert = new QPushButton("插入(输入值和位置)");
    auto* btnErase = new QPushButton("删除(输入位置)");
    hb1->addWidget(new QLabel("值:"));
    hb1->addWidget(seqlistValue);
    hb1->addWidget(new QLabel("位置:"));
    hb1->addWidget(seqlistPosition);
    hb1->addWidget(btnInsert);
    hb1->addWidget(btnErase);

    v->addStretch(1);

    connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::seqlistBuild);
    connect(btnClear, &QPushButton::clicked,this,&MainWindow::seqlistClear);
    connect(btnInsert, &QPushButton::clicked,this,&MainWindow::seqlistInsert);
    connect(btnErase, &QPushButton::clicked,this,&MainWindow::seqlistErase);
    return page;
}

QWidget* MainWindow::buildLinklistPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* f = new QFormLayout;
    v->addLayout(f);

    linklistInput = new QLineEdit;
    linklistInput->setPlaceholderText("例如: 1 3 5 7");
    f->addRow("初始序列", linklistInput);

    auto* hb0 = new QHBoxLayout;
    v->addLayout(hb0);
    auto* btnRebuild = new QPushButton("建立");
    auto* btnClear = new QPushButton("清空");
    hb0->addWidget(btnRebuild);
    hb0->addWidget(btnClear);

    auto* hb1 = new QHBoxLayout;
    v->addLayout(hb1);
    linklistValue = new QLineEdit; linklistValue->setPlaceholderText("值");
    linklistPosition = new QSpinBox; linklistPosition->setRange(0, 1000000);
    auto* btnIns = new QPushButton("插入(输入值和位置)");
    auto* btnDel = new QPushButton("删除(输入位置)");
    hb1->addWidget(new QLabel("值:"));
    hb1->addWidget(linklistValue);
    hb1->addWidget(new QLabel("位置:"));
    hb1->addWidget(linklistPosition);
    hb1->addWidget(btnIns);
    hb1->addWidget(btnDel);

    v->addStretch(1);

    connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::linklistBuild);
    connect(btnClear, &QPushButton::clicked,this,&MainWindow::linklistClear);
    connect(btnIns, &QPushButton::clicked,this,&MainWindow::linklistInsert);
    connect(btnDel, &QPushButton::clicked,this,&MainWindow::linklistErase);
    return page;
}

QWidget* MainWindow::buildStackPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* f = new QFormLayout;
    v->addLayout(f);

    stackInput = new QLineEdit;
    stackInput->setPlaceholderText("例如: 1 3 5 7");
    f->addRow("初始序列", stackInput);

    auto* hb0 = new QHBoxLayout;
    v->addLayout(hb0);
    auto* btnRebuild = new QPushButton("建立");
    auto* btnClear = new QPushButton("清空");
    hb0->addWidget(btnRebuild);
    hb0->addWidget(btnClear);

    auto* hb1 = new QHBoxLayout;
    v->addLayout(hb1);
    stackValue = new QLineEdit; stackValue->setPlaceholderText("值");
    auto* btnPush = new QPushButton("入栈");
    auto* btnPop = new QPushButton("出栈");
    hb1->addWidget(new QLabel("值:"));
    hb1->addWidget(stackValue);
    hb1->addWidget(btnPush);
    hb1->addWidget(btnPop);

    v->addStretch(1);

    connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::stackBuild);
    connect(btnClear, &QPushButton::clicked,this,&MainWindow::stackClear);
    connect(btnPush, &QPushButton::clicked,this,&MainWindow::stackPush);
    connect(btnPop, &QPushButton::clicked,this,&MainWindow::stackPop);
    return page;
}

QWidget* MainWindow::buildBTPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* f = new QFormLayout;
    v->addLayout(f);

    btInput = new QLineEdit;
    btInput->setPlaceholderText("层序，空位用哨兵表示，如: 15 6 23 4 7 17 71 5 -1 -1 50");
    btNull  = new QSpinBox; btNull->setRange(-1000000,1000000); btNull->setValue(-1);
    f->addRow("层序数组", btInput);
    f->addRow("空位哨兵", btNull);

    auto* hb = new QHBoxLayout;
    v->addLayout(hb);
    auto* btnBuild = new QPushButton("建立");
    auto* btnClear = new QPushButton("清空");
    hb->addWidget(btnBuild);
    hb->addWidget(btnClear);

    auto* hb2 = new QHBoxLayout;
    v->addLayout(hb2);
    auto* btnPre  = new QPushButton("先序遍历");
    auto* btnIn   = new QPushButton("中序遍历");
    auto* btnPost = new QPushButton("后序遍历");
    hb2->addWidget(btnPre);
    hb2->addWidget(btnIn);
    hb2->addWidget(btnPost);

    v->addStretch(1);

    connect(btnBuild,&QPushButton::clicked,this,&MainWindow::btBuild);
    connect(btnClear,&QPushButton::clicked,this,&MainWindow::btClear);
    connect(btnPre, &QPushButton::clicked, this, &MainWindow::btPreorder);
    connect(btnIn, &QPushButton::clicked, this, &MainWindow::btInorder);
    connect(btnPost,&QPushButton::clicked, this, &MainWindow::btPostorder);
    return page;
}

QWidget* MainWindow::buildBSTPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* f = new QFormLayout;
    v->addLayout(f);

    bstInput = new QLineEdit;
    bstInput->setPlaceholderText("例如: 15 6 23 4 7 17 71");
    f->addRow("初始序列", bstInput);

    auto* hb0 = new QHBoxLayout;
    v->addLayout(hb0);
    auto* btnBuild = new QPushButton("建立");
    auto* btnClear = new QPushButton("清空");
    hb0->addWidget(btnBuild);
    hb0->addWidget(btnClear);

    auto* hb1 = new QHBoxLayout;
    v->addLayout(hb1);
    bstValue = new QLineEdit; bstValue->setPlaceholderText("键值");
    auto* btnFind = new QPushButton("查找");
    auto* btnDel = new QPushButton("删除");
    hb1->addWidget(new QLabel("值:"));
    hb1->addWidget(bstValue);
    hb1->addWidget(btnFind);
    hb1->addWidget(btnDel);

    v->addStretch(1);

    connect(btnBuild,&QPushButton::clicked,this,&MainWindow::bstBuild);
    connect(btnClear,&QPushButton::clicked,this,&MainWindow::bstClear);
    connect(btnFind,&QPushButton::clicked,this,&MainWindow::bstFind);
    connect(btnDel,&QPushButton::clicked,this,&MainWindow::bstErase);
    return page;
}

QWidget* MainWindow::buildHuffmanPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* f = new QFormLayout;
    v->addLayout(f);

    huffmanInput = new QLineEdit;
    huffmanInput->setPlaceholderText("权值序列，如: 5 7 2 9 3");
    f->addRow("权值序列", huffmanInput);

    auto* hb = new QHBoxLayout;
    v->addLayout(hb);
    auto* btnBuild = new QPushButton("建立");
    auto* btnClear = new QPushButton("清空");
    hb->addWidget(btnBuild);
    hb->addWidget(btnClear);

    v->addStretch(1);

    connect(btnBuild,&QPushButton::clicked,this,&MainWindow::huffmanBuild);
    connect(btnClear,&QPushButton::clicked,this,&MainWindow::huffmanClear);
    return page;
}

// ========== 公共工具 ==========
QVector<int> MainWindow::parseIntList(const QString& text) const {
    QVector<int> out; out.reserve(64);
    QRegularExpression re("[-+]?\\d+");
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        out.push_back(m.captured(0).toInt());
    }
    return out;
}

// ========== 播放器 ==========
void MainWindow::playSteps(){
    if (stepIndex < steps.size()){
        steps[stepIndex++]();
    }else{
        timer.stop();
        statusBar()->showMessage("播放结束");
    }
}

// ========== 交互：顺序表 ==========
void MainWindow::seqlistBuild(){
    seq.clear();
    auto a = parseIntList(seqlistInput->text());
    timer.stop();
    steps.clear();
    stepIndex = 0;
    view->resetScene();
    view->setTitle("顺序表：建立");
    steps.push_back([this](){
        drawSeqlist(seq);
        statusBar()->showMessage("顺序表：开始建立");
    });
    for (int x : a){
        steps.push_back([this, x](){
            seq.insert(seq.size(), x);
            drawSeqlist(seq);
            statusBar()->showMessage(QString("顺序表：插入 %1").arg(x));
        });
    }
    timer.start();
}

// ========= 顺序表：插入（逐帧右移 + 落位） =========
void MainWindow::seqlistInsert(){
    int pos = seqlistPosition->value();
    bool ok = false;
    int val = seqlistValue->text().toInt(&ok);
    if(!ok) { statusBar()->showMessage("顺序表：请输入有效的值"); return; }

    const int n = seq.size();
    if (pos < 0) pos = 0;
    if (pos > n) pos = n;

    // 当前数组快照（字符串便于显示空格）
    QVector<QString> arr(n);
    for (int i=0;i<n;++i) arr[i] = QString::number(seq.get(i));

    // ——几何常量（按值捕获，避免悬空引用）——
    const qreal cellW = 68, cellH = 54, gap = 14;
    const qreal startX = 80, startY = 180;

    // 动画节奏
    const int prevInterval = timer.interval();
    int frames = 10;
    if (n - pos > 300) frames = 6;
    if (n - pos > 800) frames = 4;
    const int animInterval = 60;

    timer.stop(); steps.clear(); stepIndex = 0;

    // —— Step 1：高亮 [pos..n]，整体右移一格（移动“整块方框+数字”），插入位清空 —— //
    for (int f = 0; f <= frames; ++f) {
        const double t  = double(f) / frames;
        const double dx = t * (cellW + gap);       // 向右平移

        // 底层格子：n+1 个（为插入后的容量多一格）
        // 规则：对 source 区间 [pos..n-1] —— “不画静态方框”（清除原有方块）
        //      其它位置（含 pos 的目的空位、以及末尾 n）画普通底色方框。
        const int m = n + 1;

        steps.push_back([this, arr, pos, n, dx, m,
                         cellW, cellH, gap, startX, startY, f, frames, animInterval](){
            if (f == 0) timer.setInterval(animInterval); // 进入动画节奏

            view->resetScene();
            view->setTitle(QString("顺序表：插入 Step 1（pos=%1）").arg(pos));

            // 1) 画底层格子（跳过 source 区间 [pos..n-1] 以“清除原有方块”）
            for (int i=0;i<m;++i){
                const bool isSource = (i >= pos && i <= n-1);
                if (!isSource){
                    qreal x = startX + i*(cellW+gap), y = startY;
                    view->Scene()->addRect(QRectF(x,y,cellW,cellH),
                                           QPen(QColor("#5f6c7b"),2),
                                           QBrush(QColor("#e8eef9")));
                    // 左侧未受影响区（<pos）的文本直接落在格子里
                    if (i < pos && i < arr.size() && !arr[i].isEmpty()){
                        auto* t = view->Scene()->addText(arr[i]); t->setDefaultTextColor(Qt::black);
                        auto r = t->boundingRect(); t->setPos(x + (cellW-r.width())/2, y + (cellH-r.height())/2 - 1);
                    }
                }
                // 索引（全部显示，便于定位）
                qreal x = startX + i*(cellW+gap), y = startY;
                auto* idx = view->Scene()->addText(QString::number(i));
                idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(x+cellW/2-6, y+cellH+6);
            }

            // 2) 运动：把 arr[pos..n-1] 作为“整块方框+文本”整体右移 dx（高亮）
            for (int i=pos;i<n;++i){
                qreal sx = startX + i*(cellW+gap) + dx;
                qreal sy = startY;
                view->Scene()->addRect(QRectF(sx,sy,cellW,cellH),
                                       QPen(QColor("#5f6c7b"),2),
                                       QBrush(QColor("#ffd166"))); // 高亮移动元素
                auto* moving = view->Scene()->addText(arr[i]);
                moving->setDefaultTextColor(Qt::black);
                auto r = moving->boundingRect();
                moving->setPos(sx + (cellW-r.width())/2, sy + (cellH-r.height())/2 - 1);
            }

            // 插入位（pos）清空：因已跳过 source 方框绘制，视觉即为空位
            statusBar()->showMessage("顺序表：插入 Step 1 - 高亮并整体右移（原位方块已清除）");
        });
    }

    // —— Step 2：仅在 pos 放入新值并高亮（同时真实写入），其余正常底色 —— //
    {
        QVector<QString> finalShow(n+1);
        for(int i=0;i<pos;++i) finalShow[i] = arr[i];
        for(int i=pos;i<n;++i) finalShow[i+1] = arr[i];
        finalShow[pos] = QString::number(val);

        const int hiL = pos, hiR = pos;

        steps.push_back([this, finalShow, hiL, hiR,
                         cellW, cellH, gap, startX, startY, pos, val, prevInterval](){
            // 真实写入
            seq.insert(pos, val);

            view->resetScene();
            view->setTitle(QString("顺序表：插入 Step 2（pos=%1, val=%2）").arg(pos).arg(val));
            const int m = finalShow.size();
            for (int i=0;i<m;++i){
                const bool hi = (i>=hiL && i<=hiR);
                qreal x = startX + i*(cellW+gap), y = startY;
                view->Scene()->addRect(QRectF(x,y,cellW,cellH),
                                       QPen(QColor("#5f6c7b"),2),
                                       QBrush(hi? QColor("#ffd166"): QColor("#e8eef9")));
                auto* t = view->Scene()->addText(finalShow[i]); t->setDefaultTextColor(Qt::black);
                auto r = t->boundingRect(); t->setPos(x + (cellW-r.width())/2, y + (cellH-r.height())/2 - 1);
                auto* idx = view->Scene()->addText(QString::number(i));
                idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(x+cellW/2-6, y+cellH+6);
            }
            statusBar()->showMessage("顺序表：插入 Step 2 - 写入新值并高亮插入位");
            timer.setInterval(prevInterval); // 恢复节奏
        });
    }

    timer.start();
}


// ========= 顺序表：删除（逐帧左移 + 收尾） =========
void MainWindow::seqlistErase(){
    int pos = seqlistPosition->value();
    const int n = seq.size();
    if (pos < 0 || pos >= n) { statusBar()->showMessage("顺序表：删除失败(位置越界)"); return; }

    QVector<QString> arr(n);
    for (int i=0;i<n;++i) arr[i] = QString::number(seq.get(i));

    const qreal cellW = 68, cellH = 54, gap = 14;
    const qreal startX = 80, startY = 180;

    const int prevInterval = timer.interval();
    int frames = 10;
    if (n - 1 - pos > 300) frames = 6;
    if (n - 1 - pos > 800) frames = 4;
    const int animInterval = 60;

    timer.stop(); steps.clear(); stepIndex=0;

    // —— Step 1：清空 pos 文本并高亮该空格（不移动） —— //
    {
        QVector<QString> step1 = arr;
        step1[pos] = "";
        const int hiL = pos, hiR = pos;

        steps.push_back([this, step1, hiL, hiR,
                         cellW, cellH, gap, startX, startY, pos, animInterval](){
            timer.setInterval(animInterval); // 进入动画节奏

            view->resetScene(); view->setTitle(QString("顺序表：删除 Step 1（pos=%1）").arg(pos));
            const int m = step1.size();
            for (int i=0;i<m;++i){
                const bool hi = (i>=hiL && i<=hiR);
                qreal x = startX + i*(cellW+gap), y = startY;
                view->Scene()->addRect(QRectF(x,y,cellW,cellH),
                                       QPen(QColor("#5f6c7b"),2),
                                       QBrush(hi? QColor("#ffd166"): QColor("#e8eef9")));
                auto* t = view->Scene()->addText(step1[i]); t->setDefaultTextColor(Qt::black);
                auto r=t->boundingRect(); t->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
                auto* idx = view->Scene()->addText(QString::number(i));
                idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(x+cellW/2-6, y+cellH+6);
            }
            statusBar()->showMessage("顺序表：删除 Step 1 - 清空目标格并高亮");
        });
    }

    // —— Step 2：整体左移动画；源位置方框清除；最后一帧只剩 n-1 个格 —— //
    for (int f = 0; f <= frames; ++f) {
        const double t  = double(f) / frames;
        const double dx = -t * (cellW + gap);
        const bool lastFrame = (f == frames);

        steps.push_back([this, arr, pos, n, dx, lastFrame,
                         cellW, cellH, gap, startX, startY, prevInterval](){
            view->resetScene();
            view->setTitle(QString("顺序表：删除 Step 2（pos=%1）").arg(pos));

            if (!lastFrame) {
                // 中间帧：画静态方框，但“清除源方框”——跳过 [pos+1..n-1]
                for (int i=0;i<n;++i){
                    const bool isSource = (i >= pos+1 && i <= n-1);
                    if (!isSource){
                        qreal x = startX + i*(cellW+gap), y = startY;
                        view->Scene()->addRect(QRectF(x,y,cellW,cellH),
                                               QPen(QColor("#5f6c7b"),2),
                                               QBrush(QColor("#e8eef9")));
                        // 左侧未受影响区（<pos）的文本仍在格子里显示
                        if (i < pos){
                            auto* t = view->Scene()->addText(arr[i]); t->setDefaultTextColor(Qt::black);
                            auto r=t->boundingRect(); t->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
                        }
                    }
                    // 索引（全部显示）
                    qreal x = startX + i*(cellW+gap), y = startY;
                    auto* idx = view->Scene()->addText(QString::number(i));
                    idx->setDefaultTextColor(Qt::darkGray);
                    idx->setPos(x+cellW/2-6, y+cellH+6);
                }

                // 运动方块：arr[pos+1..n-1] 左移 dx（高亮移动元素，外框跟着数字）
                for (int i=pos+1;i<n;++i){
                    qreal sx = startX + i*(cellW+gap) + dx;
                    qreal sy = startY;
                    view->Scene()->addRect(QRectF(sx,sy,cellW,cellH),
                                           QPen(QColor("#5f6c7b"),2),
                                           QBrush(QColor("#ffd166"))); // 高亮移动元素
                    auto* moving = view->Scene()->addText(arr[i]);
                    moving->setDefaultTextColor(Qt::black);
                    auto r=moving->boundingRect();
                    moving->setPos(sx+(cellW-r.width())/2, sy+(cellH-r.height())/2-1);
                }
                statusBar()->showMessage("顺序表：删除 Step 2 - 整体前移中（源方框已清除）");
            } else {
                // 最后一帧：真正删除 + 只画 n-1 个格（末尾方框已移除）
                seq.erase(pos);

                QVector<QString> finalShow(n-1);
                for(int i=0;i<pos;++i) finalShow[i] = arr[i];
                for(int i=pos;i<n-1;++i) finalShow[i] = arr[i+1];

                for (int i=0;i<finalShow.size();++i){
                    bool hi = (i>=pos && i<=n-2); // 被移动后的所在格高亮
                    qreal x = startX + i*(cellW+gap), y = startY;
                    view->Scene()->addRect(QRectF(x,y,cellW,cellH),
                                           QPen(QColor("#5f6c7b"),2),
                                           QBrush(hi? QColor("#ffd166"): QColor("#e8eef9")));
                    auto* t = view->Scene()->addText(finalShow[i]); t->setDefaultTextColor(Qt::black);
                    auto r=t->boundingRect(); t->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
                    auto* idx = view->Scene()->addText(QString::number(i));
                    idx->setDefaultTextColor(Qt::darkGray);
                    idx->setPos(x+cellW/2-6, y+cellH+6);
                }
                statusBar()->showMessage("顺序表：删除 Step 2 - 已前移并移除多余方框");
                timer.setInterval(prevInterval); // 恢复节奏
            }
        });
    }

    timer.start();
}



// ========= 顺序表：清空（保持原逻辑） =========
void MainWindow::seqlistClear() {
    seq.clear();
    drawSeqlist(seq);
    statusBar()->showMessage("顺序表：已清空");
}

//交互：链表
void MainWindow::linklistBuild(){
    link.clear();
    auto a = parseIntList(linklistInput->text());
    timer.stop();
    steps.clear();
    stepIndex = 0;
    view->resetScene();
    view->setTitle("单链表：建立");
    steps.push_back([this](){
        drawLinklist(link);
        statusBar()->showMessage("单链表：开始建立");
    });
    for (int x : a){
        steps.push_back([this, x](){
            link.insert(link.size(), x);
            drawLinklist(link);
            statusBar()->showMessage(QString("单链表：插入 %1").arg(x));
        });
    }
    timer.start();
}
void MainWindow::linklistInsert() {
    int pos = linklistPosition->value();
    bool ok = false;
    int v = linklistValue->text().toInt(&ok);
    if(!ok) {
        statusBar()->showMessage("链表：请输入有效的值");
        return;
    }

    int n = link.size();
    if (pos < 0) pos = 0;
    if (pos > n) pos = n;

    QVector<int> vals; vals.reserve(n);
    for (int i = 0; i < n; ++i) vals.push_back(link.get(i));

    const qreal y = 220;
    const qreal dx = 120;
    const qreal startX = 120;
    QVector<QPointF> centers; centers.reserve(n);
    for (int i = 0; i < n; ++i) centers.push_back(QPointF(startX + i*dx, y));
    QPointF qCenter(startX + pos*dx, y + 120);

    const int prevIndex = pos - 1;
    const int succIndex = (pos < n) ? pos : -1;

    timer.stop();
    steps.clear();
    stepIndex = 0;

    auto drawBase = [this, vals, centers, y](int highlightIndex, int skipEdgeFrom){
        view->resetScene();
        view->setTitle("单链表：插入演示");
        for (int i = 0; i < vals.size(); ++i){
            bool hl = (i == highlightIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);
            if (i > 0 && (i - 1) != skipEdgeFrom){
                view->addEdge(QPointF(centers[i-1].x()+34, y),
                              QPointF(centers[i].x()-34, y));
            }
        }
    };

    steps.push_back([=](){
        drawBase(prevIndex, -1);
        view->addNode(qCenter.x(), qCenter.y(), QString::number(v), true);
        statusBar()->showMessage("链表：插入前");
    });

    steps.push_back([=](){
        drawBase(prevIndex, -1);
        view->addNode(qCenter.x(), qCenter.y(), QString::number(v), true);
        if (succIndex != -1){
            view->addEdge(QPointF(qCenter.x()+34, qCenter.y()),QPointF(centers[succIndex].x()-34, y));
            view->setTitle("单链表：设置 q->next 为 succ");
        }else{
            view->setTitle("单链表：设置 q->next 为 null");
        }
        statusBar()->showMessage("链表：步骤1 q->next=succ");
    });

    steps.push_back([=](){
        if (prevIndex >= 0){
            drawBase(prevIndex, prevIndex);
            view->addNode(qCenter.x(), qCenter.y(), QString::number(v), true);
            if (succIndex != -1){
                view->addEdge(QPointF(qCenter.x()+34, qCenter.y()),QPointF(centers[succIndex].x()-34, y));
            }
            view->addEdge(QPointF(centers[prevIndex].x()+34, y),QPointF(qCenter.x()-34, qCenter.y()));
            view->setTitle("单链表：设置 p->next 为 q");
        }else{
            drawBase(-1, -1);
            view->addNode(qCenter.x(), qCenter.y(), QString::number(v), true);
            if (succIndex != -1){
                view->addEdge(QPointF(qCenter.x()+34, qCenter.y()),QPointF(centers[succIndex].x()-34, y));
            }
            view->setTitle("单链表：设置 head = q");
        }
        statusBar()->showMessage("链表：步骤2 p->next=q / head=q");
    });

    steps.push_back([this, pos, v](){
        link.insert(pos, v);
        drawLinklist(link);
        view->setTitle("单链表：插入完成");
        statusBar()->showMessage(QString("链表：insert(%1,%2)").arg(pos).arg(v));
    });

    timer.start();
}
void MainWindow::linklistErase() {
    int pos = linklistPosition->value();
    int n = link.size();
    if (pos < 0 || pos >= n) {
        statusBar()->showMessage("链表：删除失败(位置越界)");
        return;
    }

    QVector<int> vals; vals.reserve(n);
    for (int i = 0; i < n; ++i) vals.push_back(link.get(i));

    const qreal y = 220;
    const qreal dx = 120;
    const qreal startX = 120;
    QVector<QPointF> centers; centers.reserve(n);
    for (int i = 0; i < n; ++i) centers.push_back(QPointF(startX + i*dx, y));

    const int prevIndex = pos - 1;
    const int qIndex    = pos;
    const int succIndex = (pos + 1 < n) ? pos + 1 : -1;

    timer.stop();
    steps.clear();
    stepIndex = 0;

    auto drawBase = [this, vals, centers, y](int highlightIndex, int skipEdgeFrom){
        view->resetScene();
        view->setTitle("单链表：删除演示");
        for (int i = 0; i < vals.size(); ++i){
            bool hl = (i == highlightIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);
            if (i > 0 && (i - 1) != skipEdgeFrom){
                view->addEdge(QPointF(centers[i-1].x()+34, y),
                              QPointF(centers[i].x()-34, y));
            }
        }
    };

    // 步骤0：删除前（高亮 q）
    steps.push_back([=](){
        drawBase(qIndex, -1);
        view->setTitle("单链表：删除前（高亮 q）");
        statusBar()->showMessage("链表：删除前");
    });

    // 步骤1：p->next = q->next（用弧形箭头）
    steps.push_back([=](){
        drawBase(qIndex, prevIndex); // 隐藏 p->next 旧边
        if (prevIndex >= 0) {
            if (succIndex != -1) {
                QPointF ps(centers[prevIndex].x()+34, y);
                QPointF pe(centers[succIndex].x()-34, y);
                QPointF c1(ps.x()+40, ps.y()-120);
                QPointF c2(pe.x()-40, ps.y()-120);
                view->addCurveArrow(ps, c1, c2, pe);   // <<< 弧形箭头
            }
            view->setTitle("单链表：p->next = q->next");
        } else {
            view->setTitle("单链表：head = head->next");
        }
        statusBar()->showMessage("链表：步骤1 重新连接指针");
    });

    // 步骤2：真正删除
    steps.push_back([this, pos](){
        link.erase(pos);
        drawLinklist(link);
        view->setTitle("单链表：删除完成");
        statusBar()->showMessage(QString("链表：erase(%1)").arg(pos));
    });

    timer.start();
}


void MainWindow::linklistClear() {
    link.clear();
    drawLinklist(link);
    statusBar()->showMessage("链表：已清空");
}

// 交互：栈
void MainWindow::stackBuild(){
    st.clear();
    auto a = parseIntList(stackInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene();
    view->setTitle("栈：建立");
    steps.push_back([this](){
        drawStack(st);
        statusBar()->showMessage("栈：开始建立");
    });
    for (int x : a){
        steps.push_back([this, x](){
            st.push(x);
            drawStack(st);
            statusBar()->showMessage(QString("栈：插入 %1").arg(x));
        });
    }
    timer.start();
}
void MainWindow::stackPush() {
    bool ok = false; int v = stackValue->text().toInt(&ok);
    if(!ok) {
        statusBar()->showMessage("栈：请输入有效的值");
        return;
    }
    st.push(v);
    drawStack(st);
    statusBar()->showMessage(QString("栈：push(%1)").arg(v));
}
void MainWindow::stackPop() {
    int out=0;
    if(!st.pop(&out)) {
        statusBar()->showMessage("栈：空栈，无法出栈");
        return;
    }
    drawStack(st);
    statusBar()->showMessage(QString("栈：%1出栈").arg(out));
}
void MainWindow::stackClear() {
    st.clear();
    drawStack(st);
    statusBar()->showMessage("栈：已清空");
}

//交互：二叉树
void MainWindow::btBuild(){
    auto a = parseIntList(btInput->text());
    int sent = btNull->value();
    timer.stop();
    steps.clear();
    stepIndex = 0;

    steps.push_back([this](){
        bt.clear();
        view->resetScene();
        view->setTitle("二叉树：开始建立（空树）");
        drawBT(bt.root(), 600, 120, 300, 0);
        statusBar()->showMessage("二叉树：开始建立（空树）");
    });

    for (int i = 0; i < a.size(); ++i){
        steps.push_back([this, a, sent, i](){
            QVector<int> b(a.size(), sent);
            for (int k = 0; k <= i; k++)
                b[k] = a[k];
            bt.clear();
            bt.buildTree(b.data(), b.size(), sent);

            view->resetScene();
            QString msg = (a[i]==sent) ? QString("空位(哨兵 %1) 跳过").arg(sent) : QString("插入 %1").arg(a[i]);
            view->setTitle(QString("二叉树：%1 / 共 %2 步").arg(msg).arg(a.size()));
            drawBT(bt.root(), 600, 120, 300, 0);
            statusBar()->showMessage(QString("二叉树：步骤 %1/%2，%3").arg(i+1).arg(a.size()).arg(msg));
        });
    }
    timer.start();
}
void MainWindow::btClear() {
    bt.clear();
    view->resetScene();
    view->setTitle("二叉树（空）");
    statusBar()->showMessage("二叉树：已清空");
}
void MainWindow::btPreorder() {
    // 先获取需要的长度，再分配数组并写入
    int need = bt.preorder(nullptr, 0);
    if (need <= 0) {
        view->resetScene(); view->setTitle("先序遍历：空树");
        drawBT(bt.root(), 600, 120, 300, -99999);
        return;
    }
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.preorder(buf.get(), need);
    animateBTOrder(buf.get(), n, "先序遍历");
}

void MainWindow::btInorder() {
    int need = bt.inorder(nullptr, 0);
    if (need <= 0) {
        view->resetScene(); view->setTitle("中序遍历：空树");
        drawBT(bt.root(), 600, 120, 300, -99999);
        return;
    }
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.inorder(buf.get(), need);
    animateBTOrder(buf.get(), n, "中序遍历");
}

void MainWindow::btPostorder() {
    int need = bt.postorder(nullptr, 0);
    if (need <= 0) {
        view->resetScene(); view->setTitle("后序遍历：空树");
        drawBT(bt.root(), 600, 120, 300, -99999);
        return;
    }
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.postorder(buf.get(), need);
    animateBTOrder(buf.get(), n, "后序遍历");
}

// =================== ★ 新增：通用动画函数（仅消费 C 数组） ===================

void MainWindow::animateBTOrder(const int* order, int n, const QString& title) {
    if (!order || n <= 0) {
        view->resetScene();
        view->setTitle(title + "：空树");
        drawBT(bt.root(), 600, 120, 300, -99999);
        return;
    }

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 开始帧
    steps.push_back([this, title, n](){
        view->resetScene();
        view->setTitle(QString("%1：开始（共 %2 步）").arg(title).arg(n));
        drawBT(bt.root(), 600, 120, 300, -99999);
        statusBar()->showMessage("二叉树：" + title + " 开始");
    });

    // 逐步高亮
    for (int i = 0; i < n; ++i) {
        const int key = order[i];
        steps.push_back([this, title, key, i, n](){
            view->resetScene();
            view->setTitle(QString("%1：访问 %2（第 %3/%4 步）")
                               .arg(title).arg(key).arg(i+1).arg(n));
            drawBT(bt.root(), 600, 120, 300, key);
        });
    }

    // 收尾（仅显示完成，不画任何序列）
    steps.push_back([this, title](){
        view->resetScene();
        view->setTitle(title + "：完成");
        drawBT(bt.root(), 600, 120, 300, -99999);
        statusBar()->showMessage("二叉树：" + title + " 完成");
    });

    timer.start();
}

//交互：二叉搜索树
void MainWindow::bstBuild() {
    auto a = parseIntList(bstInput->text());
    timer.stop();
    steps.clear();
    stepIndex = 0;
    steps.push_back([this]() {
        bst.clear();
        view->resetScene();
        view->setTitle("二叉搜索树：开始构建");
        drawBT(bst.root(), 600, 120, 300, 0);
    });
    for (int i = 0; i < a.size(); i++) {
        steps.push_back([this, a, i]() {
            bst.insert(a[i]);
            view->resetScene();
            view->setTitle(QString("二叉搜索树：插入 %1（第 %2/%3 步）").arg(a[i]).arg(i+1).arg(a.size()));
            drawBT(bst.root(), 600, 120, 300, 0);
        });
    }
    timer.start();
}
void MainWindow::bstFind() {
    bool ok = false;
    int value = bstValue->text().toInt(&ok);
    if(!ok) {
        statusBar()->showMessage("二叉搜索树：请输入有效的键值");
        return;
    }

    QVector<int> path;
    ds::BTNode* p = bst.root();
    while(p){
        path.push_back(p->key);
        if (value < p->key)
            p = p->left;
        else if (value > p->key)
            p = p->right;
        else break;
    }
    bool found = (!path.isEmpty() && path.last() == value);

    timer.stop();
    steps.clear();
    stepIndex = 0;
    if (path.isEmpty()) {
        steps.push_back([this](){
            view->resetScene();
            view->setTitle("二叉搜索树查找：空树");
            drawBT(bst.root(), 600, 120, 300, 0);
        });
        timer.start(); return;
    }
    for(int i = 0; i<path.size(); i++){
        int key = path[i];
        const bool isLast = (i + 1 == path.size());
        if(isLast){
            steps.push_back([this, value, key, found](){
                view->resetScene();
                view->setTitle(found ? QString("BST 查找 %1：找到").arg(value) : QString("BST 查找 %1：未找到").arg(value));
                drawBT(bst.root(), 600, 120, 300, key);
            });
        }else{
            steps.push_back([this, value, key](){
                view->resetScene();
                view->setTitle(QString("BST 查找 %1：访问 %2").arg(value).arg(key));
                drawBT(bst.root(), 600, 120, 300, key);
            });
        }
    }
    timer.start();
}
void MainWindow::bstErase() {
    bool ok = false;
    int value = bstValue->text().toInt(&ok);
    if(!ok) {
        statusBar()->showMessage("二叉搜索树：请输入有效的键值");
        return;
    }

    ds::BTNode* parent = nullptr;
    ds::BTNode* p = bst.root();
    while(p && p->key!=value) {
        parent = p;
        if (value < p->key)
            p = p->left;
        else
            p = p->right;
    }

    timer.stop();
    steps.clear();
    stepIndex = 0;
    if (!p) {
        steps.push_back([this,value]() {
            view->resetScene();
            view->setTitle(QString("二叉搜索树：删除 %1：未找到").arg(value));
            drawBT(bst.root(),600,120,300,0);
        });
        timer.start(); return;
    }

    auto drawPlain = [this](ds::BTNode* r, qreal x, qreal y, qreal distance, int highlight=INT_MIN){
        std::function<void(ds::BTNode*, qreal, qreal, qreal)> f;
        f = [this,highlight,&f](ds::BTNode* n,qreal x,qreal y,qreal s){
            if(!n) return;
            bool hl = (n->key == highlight);
            view->addNode(x,y, QString::number(n->key), hl);
            if(n->left) {
                qreal lx = x - s, ly=y+100;
                view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34));
                f(n->left,lx,ly,s/1.8);
            }
            if(n->right) {
                qreal rx=x+s, ry=y+100;
                view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34));
                f(n->right,rx,ry,s/1.8);
            }
        };
        f(r,x,y,distance);
    };

    steps.push_back([this,p](){
        std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
        g = [this,p,&g](ds::BTNode* n,qreal x,qreal y,qreal s){
            if(!n)
                return;
            view->addNode(x,y, QString::number(n->key));
            if(n->left){
                qreal lx=x-s, ly=y+100;
                if(!(n==p || n->left==p))
                    view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34));
                g(n->left,lx,ly,s/1.8);
            }
            if(n->right){
                qreal rx=x+s, ry=y+100;
                if(!(n==p || n->right==p))
                    view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34));
                g(n->right,rx,ry,s/1.8);
            }
        };
        view->resetScene();
        view->setTitle("二叉搜索树：删除第1步 删指针域（与 p 相连的边）");
        g(bst.root(),600,120,300);
    });

    steps.push_back([this,p,drawPlain](){
        std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
        g = [this,p,&g,drawPlain](ds::BTNode* n,qreal x,qreal y,qreal s){
            if(!n)
                return;
            if(n==p){
                if(n->left)  drawPlain(n->left,  x-s, y+100, s/1.8);
                if(n->right) drawPlain(n->right, x+s, y+100, s/1.8);
                return;
            }
            view->addNode(x,y, QString::number(n->key));
            if(n->left) {
                qreal lx=x-s, ly=y+100;
                view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34));
                g(n->left,lx,ly,s/1.8);
            }
            if(n->right) {
                qreal rx=x+s, ry=y+100;
                view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34));
                g(n->right,rx,ry,s/1.8);
            }
        };
        view->resetScene();
        view->setTitle("二叉搜索树：删除第2步 删值域（移除 p 本身）");
        g(bst.root(),600,120,300);
    });

    bool hasL = p->left!=nullptr, hasR = p->right!=nullptr;
    if (hasL && hasR) {
        ds::BTNode* a = p->left;
        while(a && a->right) a=a->right;
        steps.push_back([this,p,a,drawPlain](){
            std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
            g = [this,p,a,&g,drawPlain](ds::BTNode* n,qreal x,qreal y,qreal s){
                if(!n)
                    return;
                if(n == p){
                    if(n->left)  drawPlain(n->left,  x-s, y+100, s/1.8);
                    if(n->right) drawPlain(n->right, x+s, y+100, s/1.8);
                    return;
                }
                bool hl = (n==a);
                view->addNode(x,y, QString::number(n->key), hl);
                if(n->left) {
                    qreal lx=x-s, ly=y+100;
                    view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34));
                    g(n->left,lx,ly,s/1.8);
                }
                if(n->right) {
                    qreal rx=x+s, ry=y+100;
                    view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34));
                    g(n->right,rx,ry,s/1.8);
                }
            };
            view->resetScene();
            view->setTitle("二叉搜索树：删除第3步 标红左子树中序最后一个结点 a");
            g(bst.root(),600,120,300);
        });

        steps.push_back([this,value](){
            bst.eraseKey(value);
            view->resetScene();
            view->setTitle("二叉搜索树：删除第4步 接上并完成");
            drawBT(bst.root(),600,120,300,0);
        });
        timer.start();
        return;
    }
    steps.push_back([this,value](){
        bst.eraseKey(value);
        view->resetScene();
        view->setTitle("二叉搜索树：删除完成");
        drawBT(bst.root(),600,120,300,0);
    });
    timer.start();
}
void MainWindow::bstClear() {
    bst.clear(); view->resetScene(); view->setTitle("BST（空）");
}

//交互：哈夫曼树
void MainWindow::huffmanBuild() {
    auto w = parseIntList(huffmanInput->text());
    if (w.isEmpty()) {
        view->resetScene();
        view->setTitle("哈夫曼树：请输入权值序列");
        statusBar()->showMessage("哈夫曼树：无有效输入");
        return;
    }
    huff.clear();
    timer.stop();
    steps.clear();
    stepIndex = 0;

    // ---- 小工具：在边的中点沿法线轻微偏移，标注 "0/1" ----
    auto addEdgeLabel = [this](const QPointF& a, const QPointF& b, const QString& text, qreal offset = 12.0) {
        QPointF mid((a.x()+b.x())/2.0, (a.y()+b.y())/2.0);
        qreal vx = b.x() - a.x(), vy = b.y() - a.y();
        qreal L  = std::sqrt(vx*vx + vy*vy);
        qreal nx = 0.0, ny = -1.0;                     // 兜底朝上
        if (L > 1e-6) { nx = -vy / L; ny =  vx / L; }  // 单位法向
        QPointF pos = mid + QPointF(nx*offset, ny*offset);

        auto* t = view->Scene()->addText(text);
        t->setDefaultTextColor(QColor("#111"));
        QRectF tb = t->boundingRect();
        t->setPos(pos.x() - tb.width()/2.0, pos.y() - tb.height()/2.0);
    };

    // 节点半径（Canvas::addNode 的连线端点用了 ±34）
    const qreal R = 34;

    // ---- 递归绘制器：用 shared_ptr 自持有，避免悬空 ----
    using DrawFn = std::function<void(ds::BTNode*, qreal, qreal, qreal, const QString&, bool)>;
    auto drawHuffTree = std::make_shared<DrawFn>();
    *drawHuffTree = [this, drawHuffTree, addEdgeLabel, R](ds::BTNode* n, qreal x, qreal y, qreal dist,
                                                         const QString& prefix, bool annotateCodes) {
        if (!n) return;
        const bool isLeaf = (!n->left && !n->right);

        // 叶子（原始权值）高亮，内部默认色
        view->addNode(x, y, QString::number(n->key), isLeaf);

        // 左边：0
        if (n->left) {
            qreal lx = x - dist, ly = y + 100;
            QPointF a(x,  y + R), b(lx, ly - R);
            view->addEdge(a, b);
            addEdgeLabel(a, b, "0", 12.0);
            (*drawHuffTree)(n->left, lx, ly, dist/1.8, prefix + "0", annotateCodes);
        }
        // 右边：1
        if (n->right) {
            qreal rx = x + dist, ry = y + 100;
            QPointF a(x,  y + R), b(rx, ry - R);
            view->addEdge(a, b);
            addEdgeLabel(a, b, "1", 12.0);
            (*drawHuffTree)(n->right, rx, ry, dist/1.8, prefix + "1", annotateCodes);
        }

        // 只在“最终帧”给叶子节点上方贴码字（中间步骤不贴，避免拥挤）
        if (annotateCodes && isLeaf) {
            QString code = prefix.isEmpty() ? QString("0") : prefix; // 单节点特判
            auto* t = view->Scene()->addText(code);
            t->setDefaultTextColor(QColor("#065f46"));
            QRectF tb = t->boundingRect();
            t->setPos(x - tb.width()/2.0, y - R - 12 - tb.height());
        }
    };

    // 初始森林：每个权值一棵单结点树
    QVector<ds::BTNode*> forest;
    forest.reserve(w.size());
    for (int x : w) forest.push_back(ds::Huffman::makeNode(x));

    // 画“森林”的函数：按值捕获 drawHuffTree 的 shared_ptr
    auto drawForest = [this, drawHuffTree](const QVector<ds::BTNode*>& F, const QString& title) {
        view->resetScene();
        view->setTitle(title);
        qreal x = 150;
        for (int i = 0; i < F.size(); ++i) {
            (*drawHuffTree)(F[i], x, 120, 60, "", false); // 中间步骤不贴码字
            x += 180;
        }
    };

    // Step 0：展示初始森林
    QVector<ds::BTNode*> F0 = forest;
    steps.push_back([this, F0, drawForest]() {
        drawForest(F0, QString("哈夫曼树：初始森林（%1 棵）").arg(F0.size()));
        statusBar()->showMessage("哈夫曼树：开始构建");
    });

    // 逐步合并（每次取两个最小）
    QVector<ds::BTNode*> cur = forest;
    while (cur.size() > 1) {
        int i1 = -1, i2 = -1;
        for (int i = 0; i < cur.size(); ++i)
            if (i1 == -1 || cur[i]->key < cur[i1]->key) i1 = i;
        for (int i = 0; i < cur.size(); ++i)
            if (i != i1 && (i2 == -1 || cur[i]->key < cur[i2]->key)) i2 = i;
        if (i1 > i2) std::swap(i1, i2);

        int a = cur[i1]->key, b = cur[i2]->key;
        ds::BTNode* parent = ds::Huffman::makeNode(a + b);
        parent->left  = cur[i1];
        parent->right = cur[i2];

        QVector<ds::BTNode*> before = cur;
        QVector<ds::BTNode*> after  = cur;
        after[i1] = parent; after.remove(i2);

        steps.push_back([this, before, a, b, drawForest]() {
            drawForest(before, QString("哈夫曼树：选择最小两棵：%1 与 %2").arg(a).arg(b));
        });
        steps.push_back([this, after, a, b, parent, drawForest]() {
            drawForest(after,  QString("哈夫曼树：合并 %1 + %2 -> %3").arg(a).arg(b).arg(parent->key));
        });

        cur[i1] = parent;
        cur.remove(i2);
    }

    if (!cur.isEmpty())
        huff.rootNode = cur[0];

    // 收尾：完整树（边上 0/1，叶子上方标注码字）
    steps.push_back([this, drawHuffTree]() {
        view->resetScene();
        view->setTitle("哈夫曼树：构建完成（边标 0/1；叶子上方显示码字）");
        (*drawHuffTree)(huff.root(), 600, 120, 300, "", true);
        auto* legend = view->Scene()->addText("图例：黄色=原始叶结点   蓝绿色=内部结点（合并产生）");
        legend->setDefaultTextColor(QColor("#444"));
        legend->setPos(16, 54);
        statusBar()->showMessage("哈夫曼树：完成");
    });

    timer.start();
}
void MainWindow::huffmanClear() {
    huff.clear();
    view->resetScene();
    view->setTitle("哈夫曼树（空）");
    statusBar()->showMessage("哈夫曼树：已清空");
}

//缩放按钮
void MainWindow::onZoomIn() {
    view->zoomIn();
}
void MainWindow::onZoomOut() {
    view->zoomOut();
}
void MainWindow::onZoomFit() {
    view->zoomFit();
}
void MainWindow::onZoomReset() {
    view->zoomReset();
}

//绘制
void MainWindow::drawSeqlist(const ds::Seqlist& sl){
    view->resetScene();
    view->setTitle("顺序表");
    const int n = sl.size();

    const qreal cellW = 68, cellH = 54, gap = 14;
    const qreal startX = 80, startY = 180;

    for (int i=0;i<n;++i){
        qreal x = startX + i*(cellW+gap);
        qreal y = startY;
        QPen pen(QColor("#5f6c7b"), 2);
        QBrush brush(QColor("#e8eef9"));
        view->Scene()->addRect(QRectF(x,y,cellW,cellH), pen, brush);
        auto* label = view->Scene()->addText(QString::number(sl.get(i)));
        QRectF tb = label->boundingRect();
        label->setDefaultTextColor(Qt::black);
        label->setPos(x + (cellW - tb.width())/2, y + (cellH - tb.height())/2 - 1);

        auto* idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(Qt::darkGray);
        idx->setPos(x + cellW/2 - 6, y + cellH + 6);
    }
}
void MainWindow::drawLinklist(const ds::Linklist& ll){
    view->resetScene();
    view->setTitle("单链表");
    auto* p = ll.gethead();
    qreal x=120, y=220, lastx=-1;
    int i = 0;
    while(p) {
        view->addNode(x, y, QString::number(ll.get(i)));
        auto* idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(Qt::darkGray);
        idx->setPos(x-6, y+40);
        if(lastx > 0) view->addEdge(QPointF(lastx,y), QPointF(x-34,y));
        lastx = x+34;
        x += 120;
        p = p->next;
        i++;
    }
}
// 替换 mainwindow.cpp 中的 MainWindow::drawStack 定义
void MainWindow::drawStack(const ds::Stack& st){
    using std::max;

    view->resetScene();
    view->setTitle("顺序栈（U 型槽：自适应高度）");

    QGraphicsScene* S = view->Scene();

    // ===== U 型槽基础参数（横向不变）=====
    const qreal x0 = 380;      // 槽左上角
    const qreal y0 = 120;
    const qreal W  = 300;      // 槽总宽
    const qreal T  = 12;       // 壁厚
    const qreal innerPad = 6;  // 内侧左右留白

    // ===== 方块固定尺寸 + 间距（不随元素数改变）=====
    const qreal BLOCK_H = 32;  // ★ 固定方块高度（按需改一个常量即可）
    const qreal GAP     = 4;   // 块间极小缝隙（“挨紧”的视觉）
    const int   n       = st.size();

    // 为了空栈/小栈不至于太矮，设一个最小展示层数
    const int MIN_LEVELS = 6;
    const int levels     = max(n, MIN_LEVELS);

    // 根据层数反推槽内高度，然后得到 U 槽总高
    const qreal innerH = levels * BLOCK_H + (levels - 1) * GAP;
    const qreal H      = innerH + T + BLOCK_H;              // U 槽开口在上方，只需加底边厚度

    // ===== 绘制 U 型槽（随 n 增长的 H）=====
    QBrush wall(QColor("#334155"));  // 深灰蓝
    QPen   none(Qt::NoPen);
    S->addRect(QRectF(x0,         y0,         T, H), none, wall);      // 左壁
    S->addRect(QRectF(x0 + W - T, y0,         T, H), none, wall);      // 右壁
    S->addRect(QRectF(x0,         y0 + H - T, W, T), none, wall);      // 底边

    // “栈底”标识
    {
        auto* base = S->addText("栈底  BASE");
        base->setDefaultTextColor(QColor("#475569"));
        QRectF bb = base->boundingRect();
        base->setPos(x0 + W/2 - bb.width()/2, y0 + H + 8);
    }

    // ===== 内部尺寸与参照点 =====
    const qreal innerW = W - 2*T - 2*innerPad;
    const qreal leftX  = x0 + T + innerPad;
    const qreal bottomInnerY = y0 + H - T;  // 底边内缘 y

    // ===== 绘制方块（固定高度：BLOCK_H）=====
    QBrush boxFill(QColor("#93c5fd"));
    QBrush topFill(QColor("#60a5fa"));
    QPen   boxPen(QColor("#1e293b")); boxPen.setWidthF(1.2);

    for (int i = 0; i < n; ++i) {
        // i=0 是最底层；i=n-1 是栈顶
        const bool  isTop = (i == n - 1);
        const qreal yTop  = bottomInnerY - (i + 1) * BLOCK_H - i * GAP;

        auto* rect = S->addRect(QRectF(leftX, yTop, innerW, BLOCK_H),
                                boxPen, isTop ? topFill : boxFill);

        auto* label = S->addText(QString::number(st.get(i)));
        label->setDefaultTextColor(Qt::black);
        QRectF tb = label->boundingRect();
        label->setPos(leftX + innerW/2 - tb.width()/2,
                      yTop + BLOCK_H/2 - tb.height()/2);
    }

    // ===== 栈顶指示（TOP）=====
    // 指向栈顶块的“上边缘中点”
    const qreal yTopBlock = bottomInnerY - n * BLOCK_H - (n - 1) * GAP;
    const QPointF target(leftX + innerW/2, yTopBlock);

    auto* t = S->addText("TOP");
    t->setDefaultTextColor(QColor("#dc2626"));
    QRectF tb = t->boundingRect();
    const QPointF tagPos(x0 + W + 16, yTopBlock - tb.height()/2);
    t->setPos(tagPos);

    QPointF a(tagPos.x() + tb.width()/2, tagPos.y() + tb.height()/2);
    view->addEdge(a, target); // 画箭头

    // 可选：放宽场景边界，避免右侧箭头被裁切（不影响现有交互）
    S->setSceneRect(S->itemsBoundingRect().adjusted(-40, -40, 160, 80));
}

void MainWindow::drawBT(ds::BTNode* root, qreal x, qreal y, qreal distance, int highlightKey){
    if(!root) return;
    bool hl = (root->key == highlightKey);
    view->addNode(x, y, QString::number(root->key), hl);
    if(root->left) {
        qreal lx = x - distance, ly = y + 100;
        view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34));
        drawBT(root->left, lx, ly, distance/1.8, highlightKey);
    }
    if(root->right) {
        qreal rx = x + distance, ry = y + 100;
        view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34));
        drawBT(root->right, rx, ry, distance/1.8, highlightKey);
    }
}

