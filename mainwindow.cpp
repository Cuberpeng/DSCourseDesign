//
// Created by xiang on 25-9-30.
//
#include "mainwindow.h"
#include "tool.h"
#include <QToolBar>
#include <QAction>
#include <QStatusBar>

//UI构造
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(1200, 740);
    view = new Canvas(this);
    setCentralWidget(view);
    statusBar()->showMessage("数据结构可视化：顺序表 / 链表 / 栈 / 二叉树 / BST / 哈夫曼树");


    //右侧操作面板（交互）
    dock = new QDockWidget("操作面板", this);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);//页面可拖动
    tabs = new QTabWidget(dock);//子页面
    dock->setWidget(tabs);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    //Tab:顺序表
    {
        QWidget* w = new QWidget;
        auto* v = new QVBoxLayout(w);//垂直布局管理器
        auto* f = new QFormLayout;//左右两列
        v->addLayout(f);
        seqlistInput = new QLineEdit;//文本输入
        seqlistInput->setPlaceholderText("例如: 1 3 5 7");
        f->addRow("初始序列", seqlistInput);

        auto* hb0 = new QHBoxLayout;//按钮水平布局
        v->addLayout(hb0);
        auto* btnRebuild = new QPushButton("建立");
        hb0->addWidget(btnRebuild);
        auto* btnClear = new QPushButton("清空");
        hb0->addWidget(btnClear);
        // 操作行
        seqlistValue = new QLineEdit;
        seqlistValue->setPlaceholderText("值");
        seqlistPosition = new QSpinBox;
        seqlistPosition->setRange(0, 1000000);
        auto* hb1 = new QHBoxLayout;
        v->addLayout(hb1);
        hb1->addWidget(new QLabel("值:"));
        hb1->addWidget(seqlistValue);
        hb1->addWidget(new QLabel("位置:"));
        hb1->addWidget(seqlistPosition);
        auto* hb2 = new QHBoxLayout;
        v->addLayout(hb2);
        auto* btnInsert = new QPushButton("插入(输入值和位置)");
        hb2->addWidget(btnInsert);
        auto* btnErase  = new QPushButton("删除(输入位置)");
        hb2->addWidget(btnErase);

        connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::seqlistBuild);
        connect(btnClear,&QPushButton::clicked,this,&MainWindow::seqlistClear);
        connect(btnInsert,&QPushButton::clicked,this,&MainWindow::seqlistInsert);
        connect(btnErase,&QPushButton::clicked,this,&MainWindow::seqlistErase);

        tabs->addTab(w, "顺序表");
    }

    //Tab:链表
    {
        QWidget* w = new QWidget;
        auto* v = new QVBoxLayout(w);
        auto* f = new QFormLayout;
        v->addLayout(f);
        linklistInput = new QLineEdit;
        linklistInput->setPlaceholderText("例如: 1 3 5 7");
        f->addRow("初始序列", linklistInput);
        auto* hb0 = new QHBoxLayout;
        v->addLayout(hb0);
        auto* btnRebuild = new QPushButton("建立");
        hb0->addWidget(btnRebuild);
        auto* btnClear = new QPushButton("清空");
        hb0->addWidget(btnClear);
        // 操作行
        linklistValue = new QLineEdit;
        linklistValue->setPlaceholderText("值");
        linklistPosition = new QSpinBox;
        linklistPosition->setRange(0, 1000000);
        auto* hb1 = new QHBoxLayout;
        v->addLayout(hb1);
        hb1->addWidget(new QLabel("值:"));
        hb1->addWidget(linklistValue);
        hb1->addWidget(new QLabel("位置:"));
        hb1->addWidget(linklistPosition);
        auto* hb2 = new QHBoxLayout;
        v->addLayout(hb2);
        auto* btnIns= new QPushButton("插入(输入值和位置)");
        hb2->addWidget(btnIns);
        auto* btnDel= new QPushButton("删除(输入位置)");
        hb2->addWidget(btnDel);

        connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::linklistBuild);
        connect(btnClear,&QPushButton::clicked,this,&MainWindow::linklistClear);
        connect(btnIns,&QPushButton::clicked,this,&MainWindow::linklistInsert);
        connect(btnDel,&QPushButton::clicked,this,&MainWindow::linklistErase);

        tabs->addTab(w, "链表");
    }


    //演示播放定时器
    connect(&timer, &QTimer::timeout, this, &MainWindow::playSteps);
    timer.setInterval(500); // 每步 0.7s
}



//解析整数序列
QVector<int> MainWindow::parseIntList(const QString& text) const {
    QVector<int> out; out.reserve(64);
    QRegularExpression re("[-+]?\\d+");
    auto it = re.globalMatch(text);
    while (it.hasNext()) { auto m = it.next(); out.push_back(m.captured(0).toInt()); }
    return out;
}

// 演示播放
void MainWindow::playSteps(){
    if (stepIndex < steps.size()){
        steps[stepIndex++]();
    }else{
        timer.stop();
        statusBar()->showMessage("播放结束");
    }
}

//交互：顺序表
void MainWindow::seqlistBuild(){
    seq.clear();
    auto a = parseIntList(seqlistInput->text());

    // 准备步骤播放
    timer.stop();
    steps.clear();
    stepIndex = 0;
    view->resetScene();
    view->setTitle("顺序表：建立");

    // 第一步：空表
    steps.push_back([this](){
      drawSeqlist(seq);
      statusBar()->showMessage("顺序表：开始建立");
    });

    // 依次插入
    for (int x : a){
        steps.push_back([this, x](){
            seq.insert(seq.size(), x);
          drawSeqlist(seq);
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
    int pos = seqlistPosition->value();
    bool ok = false;
    int val = seqlistValue->text().toInt(&ok);
    if(!ok) {
        statusBar()->showMessage("顺序表：请输入有效的值");
        return;
    }
    if (!seq.insert(pos,val)) {
        statusBar()->showMessage("顺序表：插入失败(位置越界)");
        return;
    }
    drawSeqlist(seq);
    statusBar()->showMessage(QString("顺序表：在 %1 插入 %2").arg(pos).arg(val));
}
void MainWindow::seqlistErase(){
    int pos = seqlistPosition->value();
    if (!seq.erase(pos)) {
        statusBar()->showMessage("顺序表：删除失败(位置越界)");
        return;
    }
    drawSeqlist(seq);
    statusBar()->showMessage(QString("顺序表：删除位置 %1").arg(pos));
}

void MainWindow::seqlistClear() {
    seq.clear(); drawSeqlist(seq); statusBar()->showMessage("顺序表：已清空");
}


//交互：链表
void MainWindow::linklistBuild(){
    link.clear();
    auto a = parseIntList(linklistInput->text());

    // 准备步骤播放
    timer.stop();
    steps.clear();
    stepIndex = 0;
    view->resetScene();
    view->setTitle("单链表：建立");

    // 第一步：空表
    steps.push_back([this](){
      drawLinklist(link);
      statusBar()->showMessage("单链表：开始建立");
    });

    // 依次插入
    for (int x : a){
        steps.push_back([this, x](){
            link.insert(link.size(), x);
          drawLinklist(link);
          statusBar()->showMessage(QString("单链表：插入 %1").arg(x));
        });
    }

    // 开始播放
    timer.start();
    //link.clear();
    //auto a = parseIntList(linklistInput->text());
    //for (int x : a)
    //    link.push_back(x);
    //drawLinklist(link);
    //statusBar()->showMessage("链表：已从序列重建");
}
void MainWindow::linklistInsert() {
    int pos = linklistPosition->value();
    bool ok = false;
    int v = linklistValue->text().toInt(&ok);
    if(!ok) {
        statusBar()->showMessage("链表：请输入有效的值");
        return;
    }
    if(!link.insert(pos,v)) {
        statusBar()->showMessage("链表：插入失败(位置越界)");
        return;
    }
    drawLinklist(link);
    statusBar()->showMessage(QString("链表：insert(%1,%2)").arg(pos).arg(v));
}
void MainWindow::linklistErase() {
    int pos=linklistPosition->value();
    if(!link.erase(pos)) {
        statusBar()->showMessage("链表：删除失败(位置越界)");
        return;
    }
    drawLinklist(link);
    statusBar()->showMessage(QString("链表：erase(%1)").arg(pos));
}
void MainWindow::linklistClear() {
    link.clear();
    drawLinklist(link);
    statusBar()->showMessage("链表：已清空");
}





//绘制
void MainWindow::drawSeqlist(const ds::Seqlist& sl){
    view->resetScene();
    view->setTitle("顺序表");
    const int n = sl.size();
    const qreal y = 220;
    for(int i=0;i<n;i++) {
        qreal x = 120 + i*90;
        //auto *nitem = view->addNode(x,y, four(sl.get(i)));
        view->addNode(x, y, QString::number(sl.get(i)));
        auto* idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(Qt::darkGray);
        idx->setPos(x-6, y+40);
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
        if(lastx>0) view->addEdge(QPointF(lastx,y), QPointF(x-34,y));
        lastx=x+34;
        x+=120;
        p=p->next;
        i++;
        idx++;
    }
}

