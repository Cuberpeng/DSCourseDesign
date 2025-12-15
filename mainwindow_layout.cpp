//
// Created by xiang on 25-11-6.
//

#include "mainwindow.h"
#include "llmclient.h"
#include <QStatusBar>
#include <QApplication>
#include <QStyle>
#include <QScrollArea>
#include <QRegularExpression>
#include <QTime>

static inline qreal lerp(qreal a, qreal b, qreal t){ return a + (b - a) * t; }

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(1440, 960);
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

    // 分隔符
    canvasBar->addSeparator();

    // 缩放按钮
    QAction* actZoomIn  = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowUp),   QStringLiteral("放大"));
    QAction* actZoomOut = canvasBar->addAction(style()->standardIcon(QStyle::SP_ArrowDown), QStringLiteral("缩小"));
    QAction* actFit     = canvasBar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), QStringLiteral("适配"));
    QAction* actReset   = canvasBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), QStringLiteral("重置"));

    // ★ 新增：动画控制按钮
    canvasBar->addSeparator();

    // 播放暂停、重播按钮
    actAnimPlayToggle = canvasBar->addAction(style()->standardIcon(QStyle::SP_MediaPlay), QStringLiteral("播放"));
    actAnimReplay = canvasBar->addAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), QStringLiteral("重播"));

    // 速度调节滑块（左慢右快）
    canvasBar->addSeparator();
    QLabel* speedLabel = new QLabel(QStringLiteral("速度"), canvasBar);
    speedLabel->setStyleSheet("QLabel{color:white;font-weight:600;margin-left:6px;}");
    canvasBar->addWidget(speedLabel);

    // 新增：左侧“慢”文字
    QLabel* slowLabel = new QLabel(QStringLiteral("慢"), canvasBar);
    slowLabel->setStyleSheet("QLabel{color:rgba(255,255,255,0.8);font-size:10px;margin-left:4px;}");
    canvasBar->addWidget(slowLabel);

    // 连续型滑块：0 ~ 100（0 最慢，100 最快）
    animSpeedSlider = new QSlider(Qt::Horizontal, canvasBar);
    animSpeedSlider->setRange(0, 100);          // 连续型调节
    animSpeedSlider->setValue(40);              // 默认偏慢一点，比之前 3/5 接近
    animSpeedSlider->setFixedWidth(140);
    animSpeedSlider->setToolTip(QStringLiteral("调整动画播放速度（左慢右快）"));

    // ★ 美化样式 + 颜色渐变：右边越亮表示越快
    animSpeedSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "  height:6px;"
        "  border-radius:3px;"
        "  margin:0 6px;"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 #e5e7eb, stop:0.5 #bae6fd, stop:1 #0ea5e9);"
        "}"
        "QSlider::handle:horizontal {"
        "  width:14px;"
        "  height:14px;"
        "  margin:-4px 0;"
        "  border-radius:7px;"
        "  background:white;"
        "  border:2px solid #0ea5e9;"
        "}"
        "QSlider::sub-page:horizontal {"
        "  background:rgba(14,165,233,0.35);"
        "  border-radius:3px;"
        "}"
        "QSlider::add-page:horizontal {"
        "  background:transparent;"
        "}"
    );
    canvasBar->addWidget(animSpeedSlider);

    // 右侧“快”文字
    QLabel* fastLabel = new QLabel(QStringLiteral("快"), canvasBar);
    fastLabel->setStyleSheet("QLabel{color:rgba(255,255,255,0.9);font-size:10px;margin-right:4px;}");
    canvasBar->addWidget(fastLabel);


    // 文件操作信号
    connect(actOpen, &QAction::triggered, this, &MainWindow::openDoc);
    connect(actSave, &QAction::triggered, this, &MainWindow::saveDoc);
    //connect(actPng,  &QAction::triggered, this, &MainWindow::exportPNG);

    // 缩放信号
    connect(actZoomIn,  &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(actZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(actFit,     &QAction::triggered, this, &MainWindow::onZoomFit);
    connect(actReset,   &QAction::triggered, this, &MainWindow::onZoomReset);

    // 动画控制信号
    connect(actAnimPlayToggle, &QAction::triggered, this, &MainWindow::onAnimPlay);
    connect(actAnimReplay,     &QAction::triggered, this, &MainWindow::onAnimReplay);

    // 速度滑块信号
    connect(animSpeedSlider, &QSlider::valueChanged, this, &MainWindow::onAnimSpeedChanged);

    // ================= 中心整体：用一个 QWidget + QVBoxLayout 包起来 =================
    QWidget* central = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(4);
    setCentralWidget(central);

    // ================= 顶部：左右分栏 —— 左画布 / 右控制面板 =================
    splitter = new QSplitter(Qt::Horizontal, central);

    // 左：画布区域
    QWidget* canvasArea = new QWidget(splitter);
    QVBoxLayout* canvasLayout = new QVBoxLayout(canvasArea);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(0);

    view = new Canvas(canvasArea);
    canvasLayout->addWidget(view, 1);

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

    // 分割条设置（左右）
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(2);
    splitter->setStyleSheet("QSplitter::handle { background: #cbd5e1; }");
    splitter->setStretchFactor(0, 4); // 左：画布区域
    splitter->setStretchFactor(1, 1); // 右：控制面板

    QList<int> initialSizes;
    initialSizes << width() * 2 / 3 << width() * 1 / 3;
    splitter->setSizes(initialSizes);

    view->setMinimumWidth(400);
    controlPanel->setMinimumWidth(220);

    // 右侧控制面板内容
    auto* v = new QVBoxLayout(controlPanel);
    v->setContentsMargins(12, 10, 12, 10);
    v->setSpacing(10);

    // 模块选择：仅数据结构模块（不再包含“脚本 / DSL”）
    moduleCombo = new QComboBox(controlPanel);
    moduleCombo->addItems({
        QStringLiteral("顺序表"),
        QStringLiteral("链表"),
        QStringLiteral("栈"),
        QStringLiteral("二叉树"),
        QStringLiteral("二叉搜索树"),
        QStringLiteral("哈夫曼树"),
        QStringLiteral("AVL树")
    });
    moduleCombo->setStyleSheet(
        "QComboBox{padding:6px;border:2px solid #e2e8f0;border-radius:10px;background:white;}"
    );
    v->addWidget(new QLabel(QStringLiteral("模块选择："), controlPanel));
    v->addWidget(moduleCombo);

    // 右侧模块堆栈
    moduleStack = new QStackedWidget(controlPanel);
    v->addWidget(moduleStack, 1);

    // 构建每个模块页
    moduleStack->addWidget(makeScrollPage(buildSeqlistPage()));
    moduleStack->addWidget(makeScrollPage(buildLinklistPage()));
    moduleStack->addWidget(makeScrollPage(buildStackPage()));
    moduleStack->addWidget(makeScrollPage(buildBTPage()));
    moduleStack->addWidget(makeScrollPage(buildBSTPage()));
    moduleStack->addWidget(makeScrollPage(buildHuffmanPage()));
    moduleStack->addWidget(makeScrollPage(buildAVLPage()));
    // 注意：不再把 buildDSLPage() 加到 moduleStack 里

    // 切换模块：只切右侧面板 + 同步画布
    connect(moduleCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            moduleStack, &QStackedWidget::setCurrentIndex);
    connect(moduleCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModuleChanged);

    // ================= 中间：公共 DSL / NLI 区域 =================
    QWidget* dslContainer = buildDSLPage();          // 只构建界面，逻辑在 mainwindow_actions 里
    QWidget* dslScroll    = makeScrollPage(dslContainer);
    dslScroll->setMinimumHeight(160);

    // ================= 底部：操作信息栏（1 行 + 可滚动） =================
    QWidget* messageBarContainer = new QWidget(central);
    messageBarContainer->setStyleSheet(
        "QWidget{background:#f1f5f9;border-top:1px solid #e2e8f0;}"
    );
    QHBoxLayout* messageLayout = new QHBoxLayout(messageBarContainer);
    messageLayout->setContentsMargins(8, 2, 8, 2);
    messageLayout->setSpacing(8);

    QLabel* messageTitle = new QLabel(QStringLiteral("操作信息"));
    messageTitle->setStyleSheet(
        "QLabel{color:#64748b;font-weight:600;font-size:11px;}"
    );
    messageLayout->addWidget(messageTitle);

    messageBar = new QTextEdit(messageBarContainer);
    messageBar->setReadOnly(true);
    // ★ 关键修改：允许垂直滚动（有滚轮），方便查看历史
    messageBar->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    messageBar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageBar->setLineWrapMode(QTextEdit::NoWrap);
    messageBar->setFixedHeight(24);  // 仍然只占据一行高度
    messageBar->setStyleSheet(
        "QTextEdit{"
        "background:white;"
        "border:1px solid #e2e8f0;"
        "border-radius:6px;"
        "padding:2px 6px;"
        "font-size:11px;"
        "color:#475569;"
        "}"
    );
    messageLayout->addWidget(messageBar, 1);

    // ================= 新增：纵向 QSplitter，将三块上下可调 =================
    QSplitter* vSplit = new QSplitter(Qt::Vertical, central);
    vSplit->setChildrenCollapsible(false);
    vSplit->setHandleWidth(2);
    vSplit->setStyleSheet("QSplitter::handle { background: #cbd5e1; }");

    // 上：画布 + 右侧控制面板
    vSplit->addWidget(splitter);
    // 中：DSL / NLI 公共区域
    vSplit->addWidget(dslScroll);
    // 下：操作信息栏（1 行，但有滚动条）
    vSplit->addWidget(messageBarContainer);

    // 初始高度比例，和之前 centralLayout->addWidget(splitter,5/2/1) 对应
    vSplit->setStretchFactor(0, 5);
    vSplit->setStretchFactor(1, 3);
    vSplit->setStretchFactor(2, 1);

    centralLayout->addWidget(vSplit);

    // 动画计时器
    connect(&timer, &QTimer::timeout, this, &MainWindow::playSteps);
    // 根据当前滑块值设置初始速度
    if (animSpeedSlider) {
        onAnimSpeedChanged(animSpeedSlider->value());
    } else {
        timer.setInterval(500);
    }
    // 初始化动画按钮状态（刚启动时没有动画可播）
    updateAnimUiState();

    // 初始画布提示
    view->setTitle(QStringLiteral("请选择右侧模块并操作"));
    showMessage(QStringLiteral("欢迎使用数据结构可视化工具！"));

    // ====== 初始化大模型客户端（NLI → DSL）======
    llmClient = new LLMClient(this);

    connect(llmClient, &LLMClient::finished,this, &MainWindow::onLlmDslReady);
    connect(llmClient, &LLMClient::error,this, &MainWindow::onLlmError);
}

void MainWindow::showMessage(const QString& message) {
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timestamp, message);
    messageBar->append(formattedMessage);

    // 自动滚动到底部
    QTextCursor cursor = messageBar->textCursor();
    cursor.movePosition(QTextCursor::End);
    messageBar->setTextCursor(cursor);

    // 状态栏简短提示
    statusBar()->showMessage(message, 3000);
}

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
    while (it.hasNext())
        out.push_back(it.next().captured(0).toInt());
    return out;
}

void MainWindow::playSteps()
{
    if (stepIndex < steps.size()) {
        // 执行当前步骤
        steps[stepIndex++]();

        // 如果刚好执行到最后一步，立刻停掉定时器
        if (stepIndex >= steps.size()) {
            timer.stop();
            showMessage(QStringLiteral("播放结束"));
        }
    }
    // 每执行 / 结束一次，都刷新按钮状态
    updateAnimUiState();
}

//根据当前状态刷新按钮的辅助函数
void MainWindow::updateAnimUiState()
{
    // 工具栏未初始化时（比如构造早期）直接返回
    if (!actAnimPlayToggle) return;

    const bool hasSteps = !steps.isEmpty();
    const bool playing  = timer.isActive();

    // 播放 / 暂停 合并按钮
    actAnimPlayToggle->setEnabled(hasSteps);
    if (!hasSteps) {
        // 没有动画可播：灰色播放图标，不可点击
        actAnimPlayToggle->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        actAnimPlayToggle->setText(QStringLiteral("播放"));
        actAnimPlayToggle->setToolTip(QStringLiteral("当前没有可播放的动画"));
    } else {
        // 有动画：根据是否在播放切换图标和文字
        actAnimPlayToggle->setIcon(
            style()->standardIcon(
                playing ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay
            )
        );
        actAnimPlayToggle->setText(playing ? QStringLiteral("暂停")
                                           : QStringLiteral("播放"));
        actAnimPlayToggle->setToolTip(
            playing
            ? QStringLiteral("暂停当前动画")
            : QStringLiteral("播放 / 继续当前动画")
        );
    }

    // 重播：有动画且没在自动播放时允许（播放时禁用成浅色）
    if (actAnimReplay) {
        actAnimReplay->setEnabled(hasSteps && !playing);
    }
}

// 动画播放控制槽函数
// 播放 / 暂停
void MainWindow::onAnimPlay()
{
    if (steps.isEmpty()) {
        showMessage(QStringLiteral("当前没有可播放的动画"));
        updateAnimUiState();
        return;
    }

    if (timer.isActive()) {
        // 正在播放 -> 暂停
        timer.stop();
        showMessage(QStringLiteral("动画：已暂停"));
    } else {
        // 暂停 / 未播放 -> 开始 / 继续
        if (stepIndex >= steps.size()) {
            // 如果已经播完，从头开始
            stepIndex = 0;
        }
        timer.start();
        showMessage(QStringLiteral("动画：开始 / 继续播放"));
    }

    updateAnimUiState();
}

void MainWindow::onAnimReplay()
{
    if (steps.isEmpty()) {
        showMessage(QStringLiteral("当前没有可重播的动画"));
        updateAnimUiState();
        return;
    }

    // 正在播放时，本来按钮就被禁用，这里再做一层保护
    if (timer.isActive()) {
        timer.stop();
    }

    stepIndex = 0;
    showMessage(QStringLiteral("动画：从头重新播放"));

    timer.start();
    updateAnimUiState();
}


void MainWindow::onAnimSpeedChanged(int value)
{
    if (value < 0)  value = 0;
    if (value > 100) value = 100;

    // t = 0 慢端，t = 1 快端
    qreal t = value / 100.0;

    // 1）控制所有使用 QTimer 的动画（顺序表，链表，栈，普通二叉树，BST，Huffman 等）
    const int maxInterval = 800;   // 最慢：0 档
    const int minInterval = 80;    // 最快：100 档
    int interval = static_cast<int>(maxInterval + (minInterval - maxInterval) * t);
    timer.setInterval(interval);

    // 2）控制所有用 QTimeLine 的树类动画（主要是 AVL 旋转）
    const int maxDuration = 1600;  // 最慢
    const int minDuration = 300;   // 最快
    animTimelineDurationMs_ = static_cast<int>(maxDuration + (minDuration - maxDuration) * t);
}

// 缩放按钮
void MainWindow::onZoomIn()   { view->zoomIn();   }
void MainWindow::onZoomOut()  { view->zoomOut();  }
void MainWindow::onZoomFit()  { view->zoomFit();  }
void MainWindow::onZoomReset(){ view->zoomReset(); }
