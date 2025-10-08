//
// Created by xiang on 25-9-30.
//
#include "mainwindow.h"
#include "tool.h"
#include <QToolBar>
#include <QAction>
#include <QStatusBar>

// ================== 构造 & UI ==================
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(1200, 740);
    view = new Canvas(this);
    setCentralWidget(view);
    statusBar()->showMessage("数据结构可视化：顺序表 / 链表 / 栈 / 二叉树 / BST / 哈夫曼树");


    // —— 右侧操作面板（交互）——
    dock = new QDockWidget("操作面板", this);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);//页面可拖动
    tabs = new QTabWidget(dock);//子页面
    dock->setWidget(tabs);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // --- Tab: 顺序表 ---
    {
        QWidget* w = new QWidget;
        auto* v = new QVBoxLayout(w);//垂直布局管理器
        auto* f = new QFormLayout;//左右两列
        v->addLayout(f);
        seqBulkEdit = new QLineEdit;//文本输入
        seqBulkEdit->setPlaceholderText("例如: 1 3 5 7");
        f->addRow("初始序列", seqBulkEdit);

        auto* hb0 = new QHBoxLayout;//按钮水平布局
        v->addLayout(hb0);
        auto* btnRebuild = new QPushButton("建立");
        hb0->addWidget(btnRebuild);
        auto* btnClear = new QPushButton("清空");
        hb0->addWidget(btnClear);
        // 操作行
        seqValEdit = new QLineEdit;
        seqValEdit->setPlaceholderText("值");
        seqPosSpin = new QSpinBox;
        seqPosSpin->setRange(0, 1000000);
        auto* hb1 = new QHBoxLayout;
        v->addLayout(hb1);
        hb1->addWidget(new QLabel("值:"));
        hb1->addWidget(seqValEdit);
        hb1->addWidget(new QLabel("位置:"));
        hb1->addWidget(seqPosSpin);
        auto* hb2 = new QHBoxLayout;
        v->addLayout(hb2);
        auto* btnInsert = new QPushButton("插入(pos, val)");
        hb2->addWidget(btnInsert);
        auto* btnErase  = new QPushButton("删除(pos)");
        hb2->addWidget(btnErase);
        //auto* btnPush   = new QPushButton("push_back(val)");hb2->addWidget(btnPush);
        //auto* btnPop    = new QPushButton("pop_back()");    hb2->addWidget(btnPop);

        connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::seqlistBuild);
        connect(btnClear,&QPushButton::clicked,this,&MainWindow::seqlistClear);
        connect(btnInsert,&QPushButton::clicked,this,&MainWindow::seqlistInsert);
        connect(btnErase,&QPushButton::clicked,this,&MainWindow::seqlistErase);
        //connect(btnPush,&QPushButton::clicked,this,&MainWindow::seqPush);
        //connect(btnPop,&QPushButton::clicked,this,&MainWindow::seqPop);

        tabs->addTab(w, "顺序表");
    }
    // —— 演示播放定时器 ——
    connect(&timer, &QTimer::timeout, this, &MainWindow::playSteps);
    timer.setInterval(500); // 每步 0.7s
}



// ================== 公共小工具 解析整数序列==================
QVector<int> MainWindow::parseIntList(const QString& text) const {
    QVector<int> out; out.reserve(64);
    QRegularExpression re("[-+]?\\d+");
    auto it = re.globalMatch(text);
    while (it.hasNext()) { auto m = it.next(); out.push_back(m.captured(0).toInt()); }
    return out;
}

// ================== 演示播放控件（保留） ==================
void MainWindow::playSteps(){
    if (stepIndex < steps.size()){
        steps[stepIndex++]();
    }else{
        timer.stop();
        statusBar()->showMessage("播放结束");
    }
}

    // ================== 交互：顺序表 ==================
    void MainWindow::seqlistBuild(){
        seq.clear();
        auto a = parseIntList(seqBulkEdit->text());

    // 准备步骤播放
    timer.stop();
    steps.clear(); stepIndex = 0;
    view->resetScene(); view->setTitle("顺序表（逐步建立）");

    // 第一步：空表
    steps.push_back([this](){
      drawSeq(seq);
      statusBar()->showMessage("顺序表：开始建立（空表）");
    });

    // 依次插入
    for (int x : a){
        steps.push_back([this, x](){
            seq.insert(seq.size(), x);
          drawSeq(seq);
          statusBar()->showMessage(QString("顺序表：插入 %1").arg(x));
        });
    }

    // 开始播放
    timer.start();

        //for (int x : a) {
        //    seq.insert(seq.size(), x);
        //}
        //drawSeq(seq); statusBar()->showMessage("顺序表：已从序列重建");
    }
    void MainWindow::seqlistInsert(){
        int pos = seqPosSpin->value();
        bool ok=false; int val = seqValEdit->text().toInt(&ok); if(!ok){ statusBar()->showMessage("顺序表：请输入有效的值"); return; }
        if (!seq.insert(pos,val)) { statusBar()->showMessage("顺序表：插入失败(位置越界)"); return; }
        drawSeq(seq); statusBar()->showMessage(QString("顺序表：在 %1 插入 %2").arg(pos).arg(val));
    }
    void MainWindow::seqlistErase(){
        int pos = seqPosSpin->value();
        if (!seq.erase(pos)) { statusBar()->showMessage("顺序表：删除失败(位置越界)"); return; }
        drawSeq(seq); statusBar()->showMessage(QString("顺序表：删除位置 %1").arg(pos));
    }
    //void MainWindow::seqPush(){
    //    bool ok=false; int val = seqValEdit->text().toInt(&ok); if(!ok){ statusBar()->showMessage("顺序表：请输入有效的值"); return; }
    //    //seq.push_back(val);
    //    drawSeq(seq); statusBar()->showMessage(QString("顺序表：push_back(%1)").arg(val));
    //}
    //void MainWindow::seqPop(){
    //    int out=0; if(!seq.pop_back(&out)){ statusBar()->showMessage("顺序表：空表，无法 pop"); return; }
    //    drawSeq(seq); statusBar()->showMessage(QString("顺序表：pop_back() -> %1").arg(out));
    //}
    void MainWindow::seqlistClear(){ seq.clear(); drawSeq(seq); statusBar()->showMessage("顺序表：已清空"); }




// ================== 绘制助手 ==================
void MainWindow::drawSeq(const ds::Seqlist& sl){
    view->resetScene(); view->setTitle("顺序表");
    const int n = sl.size(); const qreal y = 220;
    for(int i=0;i<n;++i) {
        qreal x = 120 + i*90;
        //auto *nitem = view->addNode(x,y, four(sl.get(i)));
        view->addNode(x, y, QString::number(sl.get(i)));
        auto* idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(Qt::darkGray);
        idx->setPos(x-6, y+40);
    }
}

