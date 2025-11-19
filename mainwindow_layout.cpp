//
// Created by xiang on 25-11-6.
//
//
// mainwindow_base.cpp
// 框架与全局：窗口、分栏、工具栏、模块页容器、通用工具
//
#include "mainwindow.h"
#include <QStatusBar>
#include <QApplication>
#include <QStyle>
#include <QScrollArea>
#include <QRegularExpression>
#include <QTime>

static inline qreal lerp(qreal a, qreal b, qreal t){ return a + (b - a) * t; }

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(1280, 800);
    setWindowTitle(QStringLiteral("数据结构可视化"));
    statusBar()->showMessage(QStringLiteral("就绪"));

    // 顶部：画布工具栏（包含文件操作和缩放操作）
    canvasBar = addToolBar(QStringLiteral("画布"));
    canvasBar->setMovable(false);
    canvasBar->setStyleSheet(
        "QToolBar{background:#0ea5e9;border:0;padding:4px;} "
        "QToolButton{color:white;font-weight:600;padding:6px 10px;} "
        "QToolButton:hover{background:rgba(255,255,255,0.15);border-radius:6px;}"
    );

    // 文件操作按钮
    QAction* actOpen = canvasBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), QStringLiteral("打开"));
    QAction* actSave = canvasBar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), QStringLiteral("保存"));
    QAction* actPng = canvasBar->addAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), QStringLiteral("导出PNG"));

    // 添加分隔符
    canvasBar->addSeparator();

    // 缩放按钮
    QAction* actZoomIn  = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowUp),   QStringLiteral("放大"));
    QAction* actZoomOut = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowDown), QStringLiteral("缩小"));
    QAction* actFit     = canvasBar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), QStringLiteral("适配"));
    QAction* actReset   = canvasBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), QStringLiteral("重置"));

    // 连接文件操作信号
    connect(actOpen, &QAction::triggered, this, &MainWindow::openDoc);
    connect(actSave, &QAction::triggered, this, &MainWindow::saveDoc);
    connect(actPng,  &QAction::triggered, this, &MainWindow::exportPNG);

    // 连接缩放操作信号
    connect(actZoomIn,  &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(actZoomOut,  &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(actFit,     &QAction::triggered, this, &MainWindow::onZoomFit);
    connect(actReset,   &QAction::triggered, this, &MainWindow::onZoomReset);

    // 中心：左右分栏 —— 左画布优先展示
    splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // 左：画布区域（包含画布和信息栏）
    QWidget* canvasArea = new QWidget(splitter);
    QVBoxLayout* canvasLayout = new QVBoxLayout(canvasArea);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(0);

    // 画布
    view = new Canvas(canvasArea);
    canvasLayout->addWidget(view, 1); // 画布占据主要空间

    // 信息栏
    QWidget* messageBarContainer = new QWidget(canvasArea);
    messageBarContainer->setStyleSheet("QWidget{background:#f8fafc;border-top:1px solid #e2e8f0;}");
    QVBoxLayout* messageLayout = new QVBoxLayout(messageBarContainer);
    messageLayout->setContentsMargins(12, 8, 12, 8);

    QLabel* messageTitle = new QLabel(QStringLiteral("操作信息"));
    messageTitle->setStyleSheet("QLabel{color:#334155;font-weight:600;font-size:12px;margin-bottom:4px;}");
    messageLayout->addWidget(messageTitle);

    messageBar = new QTextEdit(messageBarContainer);
    messageBar->setMaximumHeight(120);
    messageBar->setReadOnly(true);
    messageBar->setStyleSheet(
        "QTextEdit{"
        "background:white;"
        "border:1px solid #e2e8f0;"
        "border-radius:6px;"
        "padding:8px;"
        "font-size:12px;"
        "color:#475569;"
        "}"
    );
    messageLayout->addWidget(messageBar);

    canvasLayout->addWidget(messageBarContainer);

    splitter->addWidget(canvasArea);

    // 右：控制面板（有明显的色块分区）
    controlPanel = new QWidget(splitter);
    controlPanel->setStyleSheet(
        "QWidget{background:#f8fafc;} "
        "QGroupBox{border:2px solid #e2e8f0;border-radius:10px;margin-top:8px;background:white;} "
        "QGroupBox::title{subcontrol-origin:margin; left:10px; padding:0 6px; color:#334155; background:#e2e8f0; border-radius:6px;} "
        "QPushButton.primary{background:#22c55e;color:white;border-radius:10px;padding:8px 12px;font-weight:600;} "
        "QPushButton.primary:hover{filter:brightness(1.05);} "
        "QPushButton.warn{background:#ef4444;color:white;border-radius:10px;padding:8px 12px;font-weight:600;} "
        "QPushButton.warn:hover{filter:brightness(1.05);} "
        "QPushButton.neutral{background:#3b82f6;color:white;border-radius:10px;padding:8px 12px;font-weight:600;} "
        "QPushButton.neutral:hover{filter:brightness(1.05);} "
        "QLabel{color:#334155;font-weight:600;}"
    );
    splitter->addWidget(controlPanel);

    // 设置分割器的属性，确保左右相邻显示
    splitter->setChildrenCollapsible(false); // 防止子部件被完全折叠
    splitter->setHandleWidth(2); // 设置分割条宽度
    splitter->setStyleSheet("QSplitter::handle { background: #cbd5e1; }"); // 分割条颜色

    // 设置拉伸因子和初始大小
    splitter->setStretchFactor(0, 3); // 画布拉伸因子更大
    splitter->setStretchFactor(1, 1); // 控制面板拉伸因子较小

    // 设置初始大小，确保画布有足够空间
    QList<int> initialSizes;
    initialSizes << width() * 2 / 3 << width() * 1 / 3;
    splitter->setSizes(initialSizes);

    // 设置最小尺寸
    view->setMinimumWidth(400);
    controlPanel->setMinimumWidth(280);

    // 右侧容器
    auto* v = new QVBoxLayout(controlPanel);
    v->setContentsMargins(12, 10, 12, 10);
    v->setSpacing(10);

    moduleCombo = new QComboBox(controlPanel);
    moduleCombo->addItems({QStringLiteral("顺序表"), QStringLiteral("链表"), QStringLiteral("栈"),
                           QStringLiteral("二叉树"), QStringLiteral("二叉搜索树"), QStringLiteral("哈夫曼树"), QStringLiteral("AVL树"), QStringLiteral("脚本/DSL")});
    moduleCombo->setStyleSheet("QComboBox{padding:6px;border:2px solid #e2e8f0;border-radius:10px;}");
    v->addWidget(new QLabel(QStringLiteral("模块选择："), controlPanel));
    v->addWidget(moduleCombo);

    moduleStack = new QStackedWidget(controlPanel);
    v->addWidget(moduleStack, 1);

    // 构建每个模块页（见 pages 文件）
    moduleStack->addWidget(makeScrollPage(buildSeqlistPage()));
    moduleStack->addWidget(makeScrollPage(buildLinklistPage()));
    moduleStack->addWidget(makeScrollPage(buildStackPage()));
    moduleStack->addWidget(makeScrollPage(buildBTPage()));
    moduleStack->addWidget(makeScrollPage(buildBSTPage()));
    moduleStack->addWidget(makeScrollPage(buildHuffmanPage()));
    moduleStack->addWidget(makeScrollPage(buildAVLPage()));
    moduleStack->addWidget(makeScrollPage(buildDSLPage()));

    connect(moduleCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            moduleStack, &QStackedWidget::setCurrentIndex);

    // 动画计时
    connect(&timer, &QTimer::timeout, this, &MainWindow::playSteps);
    timer.setInterval(500);

    // 初始画布提示
    view->setTitle(QStringLiteral("请选择右侧模块并操作"));
    showMessage(QStringLiteral("欢迎使用数据结构可视化工具！请从右侧选择数据结构模块开始操作。"));
}

// 新增：显示消息到信息栏
void MainWindow::showMessage(const QString& message) {
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timestamp, message);
    messageBar->append(formattedMessage);

    // 自动滚动到底部
    QTextCursor cursor = messageBar->textCursor();
    cursor.movePosition(QTextCursor::End);
    messageBar->setTextCursor(cursor);

    // 同时更新状态栏（简短提示）
    statusBar()->showMessage(message, 3000);
}

// 新增：清空信息栏
void MainWindow::clearMessages() {
    messageBar->clear();
}

QWidget* MainWindow::makeScrollPage(QWidget* content) {
    auto* sa = new QScrollArea;
    sa->setWidget(content);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    return sa;
}

QVector<int> MainWindow::parseIntList(const QString& text) const {
    QVector<int> out; out.reserve(64);
    QRegularExpression re("[-+]?\\d+");
    auto it = re.globalMatch(text);
    while (it.hasNext()) out.push_back(it.next().captured(0).toInt());
    return out;
}

void MainWindow::playSteps(){
    if (stepIndex < steps.size()){
        steps[stepIndex++]();
    }else{
        timer.stop();
        showMessage(QStringLiteral("播放结束"));
    }
}

// 缩放按钮
void MainWindow::onZoomIn()   { view->zoomIn();   }
void MainWindow::onZoomOut()  { view->zoomOut();  }
void MainWindow::onZoomFit()  { view->zoomFit();  }
void MainWindow::onZoomReset(){ view->zoomReset(); }