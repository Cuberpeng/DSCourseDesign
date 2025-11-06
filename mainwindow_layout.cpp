//
// Created by xiang on 25-11-6.
//
//
#include "mainwindow.h"
#include <QStatusBar>
#include <QApplication>
#include <QStyle>
#include <QScrollArea>
#include <QRegularExpression>

static inline qreal lerp(qreal a, qreal b, qreal t){ return a + (b - a) * t; }

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(1280, 800);
    setWindowTitle(QStringLiteral("数据结构可视化"));
    statusBar()->showMessage(QStringLiteral("就绪"));

    // 顶部：画布缩放工具栏（统一配色）
    canvasBar = addToolBar(QStringLiteral("画布"));
    canvasBar->setMovable(false);
    canvasBar->setStyleSheet(
        "QToolBar{background:#0ea5e9;border:0;padding:4px;} "
        "QToolButton{color:white;font-weight:600;padding:6px 10px;} "
        "QToolButton:hover{background:rgba(255,255,255,0.15);border-radius:6px;}"
    );
    QAction* actZoomIn  = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowUp),   QStringLiteral("放大"));
    QAction* actZoomOut = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowDown), QStringLiteral("缩小"));
    QAction* actFit     = canvasBar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), QStringLiteral("适配"));
    QAction* actReset   = canvasBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), QStringLiteral("重置"));
    connect(actZoomIn,  &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(actZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(actFit,     &QAction::triggered, this, &MainWindow::onZoomFit);
    connect(actReset,   &QAction::triggered, this, &MainWindow::onZoomReset);

    // 中心：左右分栏 —— 左画布优先展示
    splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // 左：画布（初始即完整可见）
    view = new Canvas(splitter);
    splitter->addWidget(view);

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
                           QStringLiteral("二叉树"), QStringLiteral("二叉搜索树"), QStringLiteral("哈夫曼树")});
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

    connect(moduleCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            moduleStack, &QStackedWidget::setCurrentIndex);

    // 动画计时
    connect(&timer, &QTimer::timeout, this, &MainWindow::playSteps);
    timer.setInterval(500);

    // 初始画布提示
    view->setTitle(QStringLiteral("请选择右侧模块并操作"));
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
        statusBar()->showMessage(QStringLiteral("播放结束"));
    }
}

// 缩放按钮
void MainWindow::onZoomIn()   { view->zoomIn();   }
void MainWindow::onZoomOut()  { view->zoomOut();  }
void MainWindow::onZoomFit()  { view->zoomFit();  }
void MainWindow::onZoomReset(){ view->zoomReset(); }

