//
// Created by xiang on 25-9-30.
//
#include "mainwindow.h"
//#include "tool.h"
#include <QToolBar>
#include <QAction>
#include <QStatusBar>

//UI构造
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(1200, 740);
    view = new Canvas(this);
    setCentralWidget(view);
    statusBar()->showMessage("数据结构可视化");


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

    //Tab:栈
    {
        QWidget* w = new QWidget;
        auto* v = new QVBoxLayout(w);
        auto* f = new QFormLayout;
        v->addLayout(f);
        stackInput = new QLineEdit;
        stackInput->setPlaceholderText("例如: 1 3 5 7");
        f->addRow("初始序列", stackInput);
        auto* hb0 = new QHBoxLayout;
        v->addLayout(hb0);
        auto* btnRebuild = new QPushButton("建立");
        hb0->addWidget(btnRebuild);
        auto* btnClear = new QPushButton("清空");
        hb0->addWidget(btnClear);
        //操作行
        auto* hb1 = new QHBoxLayout;
        v->addLayout(hb1);
        stackValue = new QLineEdit;
        stackValue->setPlaceholderText("值");
        auto* btnPush = new QPushButton("入栈");
        hb1->addWidget(new QLabel("值:"));
        hb1->addWidget(stackValue);
        hb1->addWidget(btnPush);
        auto* hb2 = new QHBoxLayout;
        v->addLayout(hb2);
        auto* btnPop = new QPushButton("出栈");
        hb2->addWidget(btnPop);

        // 信号连接
        connect(btnRebuild,&QPushButton::clicked,this,&MainWindow::stackBuild);
        connect(btnClear,&QPushButton::clicked,this,&MainWindow::stackClear);
        connect(btnPush,&QPushButton::clicked,this,&MainWindow::stackPush);
        connect(btnPop,&QPushButton::clicked,this,&MainWindow::stackPop);

        tabs->addTab(w, "栈");
    }

    //Tab:二叉树
    {
        QWidget* w = new QWidget;
        auto* v = new QVBoxLayout(w);
        auto* f = new QFormLayout;
        v->addLayout(f);
        btInput = new QLineEdit;
        btInput->setPlaceholderText("层序，空位用哨兵表示，如: 15 6 23 4 7 17 71 5 -1 -1 50");
        btNull = new QSpinBox;
        btNull->setRange(-1000000,1000000);
        btNull->setValue(-1);
        f->addRow("层序数组", btInput);
        f->addRow("空位哨兵", btNull);
        auto* hb = new QHBoxLayout;
        v->addLayout(hb);
        auto* btnBuild = new QPushButton("建立");
        hb->addWidget(btnBuild);
        auto* btnClear = new QPushButton("清空");
        hb->addWidget(btnClear);
        connect(btnBuild,&QPushButton::clicked,this,&MainWindow::btBuild);
        connect(btnClear,&QPushButton::clicked,this,&MainWindow::btClear);

        tabs->addTab(w, "二叉树");
    }

    // 二叉搜索树
    {
        QWidget* w = new QWidget;
        auto* v = new QVBoxLayout(w);
        auto* f = new QFormLayout;
        v->addLayout(f);
        bstInput = new QLineEdit;
        bstInput->setPlaceholderText("例如: 15 6 23 4 7 17 71");
        f->addRow("初始序列", bstInput);
        auto* hb0 = new QHBoxLayout;
        v->addLayout(hb0);
        auto* btnBuild = new QPushButton("建立");
        hb0->addWidget(btnBuild);
        auto* btnClear = new QPushButton("清空");
        hb0->addWidget(btnClear);

        auto* hb1 = new QHBoxLayout;
        v->addLayout(hb1);
        bstValue = new QLineEdit;
        bstValue->setPlaceholderText("键值");
        auto* btnFind= new QPushButton("查找");
        auto* btnDel = new QPushButton("删除");
        hb1->addWidget(new QLabel("值:"));
        hb1->addWidget(bstValue);
        hb1->addWidget(btnFind);
        hb1->addWidget(btnDel);

        connect(btnBuild,&QPushButton::clicked,this,&MainWindow::bstBuild);
        connect(btnClear,&QPushButton::clicked,this,&MainWindow::bstClear);
        connect(btnFind,&QPushButton::clicked,this,&MainWindow::bstFind);
        connect(btnDel,&QPushButton::clicked,this,&MainWindow::bstErase);

        tabs->addTab(w, "二叉搜索树");
    }

    //哈夫曼树
    {
        QWidget* w = new QWidget;
        auto* v = new QVBoxLayout(w);
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

        connect(btnBuild,&QPushButton::clicked,this,&MainWindow::huffmanBuild);
        connect(btnClear,&QPushButton::clicked,this,&MainWindow::huffmanClear);

        tabs->addTab(w, "哈夫曼树");
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

//栈
void MainWindow::stackBuild(){
    //st.clear();
    //auto a = parseIntList(stackInput->text()); // 与顺序表/链表一致的解析
    //for (int x : a) st.push(x);                   // 顺序压栈：底 1 ... 顶 为最后一个
    //drawStack(st);
    //statusBar()->showMessage("栈：已从序列重建");


    st.clear();
    auto a = parseIntList(stackInput->text());

    // 准备步骤播放
    timer.stop();
    steps.clear();
    stepIndex = 0;
    view->resetScene();
    view->setTitle("栈：建立");

    // 第一步：空表
    steps.push_back([this](){
        drawStack(st);
        statusBar()->showMessage("栈：开始建立");
    });

    // 依次插入
    for (int x : a){
        steps.push_back([this, x](){
            st.push(x);
            drawStack(st);
            statusBar()->showMessage(QString("栈：插入 %1").arg(x));
        });
    }

    // 开始播放
    timer.start();
}
void MainWindow::stackPush() {
    bool ok = false;
    int v = stackValue->text().toInt(&ok);
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

//二叉树
void MainWindow::btBuild(){
    auto a = parseIntList(btInput->text());
    int sent = btNull->value();

    // 准备步骤播放（完全仿照顺序表/链表/栈）
    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 第 0 步：空树
    steps.push_back([this](){
        bt.clear();
        view->resetScene();
        view->setTitle("二叉树：开始建立（空树）");
        drawBT(bt.root(), 600, 120, 300, 0);
        statusBar()->showMessage("二叉树：开始建立（空树）");
    });

    // 逐步“前缀重建”并绘制（≤i 用真实值，其它位置用哨兵）
    for (int i = 0; i < a.size(); ++i){
        steps.push_back([this, a, sent, i](){//sent为哨兵，表示空位
            QVector<int> b(a.size(), sent);
            for (int k = 0; k <= i; k++)
                b[k] = a[k];//b中前i个是真数，后面用哨兵代替，表示目前在进行第i个

            bt.clear();
            bt.buildTree(b.data(), b.size(), sent);

            view->resetScene();
            QString msg = (a[i]==sent) ? QString("空位(哨兵 %1) 跳过").arg(sent) : QString("插入 %1").arg(a[i]);
            view->setTitle(QString("二叉树：%1 / 共 %2 步").arg(msg).arg(a.size()));
            drawBT(bt.root(), 600, 120, 300, 0);
            statusBar()->showMessage(QString("二叉树：步骤 %1/%2，%3").arg(i+1).arg(a.size()).arg(msg));
        });
    }

    // 播放
    timer.start();
}
void MainWindow::btClear() {
    bt.clear();
    view->resetScene();
    view->setTitle("二叉树（空）");
    statusBar()->showMessage("二叉树：已清空");
}

//二叉搜索树
void MainWindow::bstBuild() {
    auto a = parseIntList(bstInput->text());
    int sent = btNull->value();
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
        steps.push_back([this, a, sent, i]() {
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
    //单次遍历：收集访问路径，并判断是否命中
    QVector<int> path;
    ds::BTNode* p = bst.root();
    while(p){
        path.push_back(p->key);
        if (value < p->key)
            p = p->left;
        else if (value > p->key)
            p = p->right;
        else
            break;
    }
    bool found = (!path.isEmpty() && path.last() == value);
    //组织播放
    timer.stop();
    steps.clear();
    stepIndex = 0;

    if (path.isEmpty()) {
        // 空树
        steps.push_back([this](){
          view->resetScene();
          view->setTitle("二叉搜索树查找：空树");
          drawBT(bst.root(), 600, 120, 300, 0);
        });
        timer.start();
        return;
    }
    // 逐步高亮访问路径
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
    //找p
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
        timer.start();
        return;
    }

    //子树绘画lambda
    auto drawPlain = [this](ds::BTNode* r, qreal x, qreal y, qreal distance, int highlight=INT_MIN){
        std::function<void(ds::BTNode*, qreal, qreal, qreal)> f;
        f = [this,highlight,&f](ds::BTNode* n,qreal x,qreal y,qreal s){
            if(!n) return;
            bool hl = (n->key == highlight);
            view->addNode(x,y, QString::number(n->key), hl);
            if(n->left) {
                qreal lx = x-s, ly=y+100;
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

    // 1)删除与 p 相连的边
    steps.push_back([this,p,drawPlain](){
      std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
      g = [this,p,&g](ds::BTNode* n,qreal x,qreal y,qreal s){
        if(!n) return;
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

    // 2)删除p结点，但保留它的左右子树
    steps.push_back([this,p,drawPlain](){
      std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
      g = [this,p,&g,drawPlain](ds::BTNode* n,qreal x,qreal y,qreal s){
        if(!n) return;
        if(n==p){
          // 不画 p，自身的左右子树分别在原相对位置绘制（不连边）
          if(n->left) {
              qreal lx=x-s, ly=y+100;
              drawPlain(n->left,lx,ly,s/1.8);
          }
          if(n->right) {
              qreal rx=x+s, ry=y+100;
              drawPlain(n->right,rx,ry,s/1.8);
          }
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
      view->resetScene(); view->setTitle("二叉搜索树：删除第2步 删值域（移除 p 本身）");
      g(bst.root(),600,120,300);
    });


    bool hasL = p->left!=nullptr, hasR=p->right!=nullptr;
    if (hasL && hasR) {
        // 3) 找左子树中序最后一个结点 a，并高亮（在“去掉 p”的画面上）
        ds::BTNode* a = p->left;
        while(a && a->right)
            a=a->right;
        steps.push_back([this,p,a,drawPlain](){
          std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
          g = [this,p,a,&g,drawPlain](ds::BTNode* n,qreal x,qreal y,qreal s){
            if(!n) return;
            if(n == p){
              if(n->left) {
                  qreal lx=x-s, ly=y+100;
                  drawPlain(n->left,lx,ly,s/1.8);
              }
              if(n->right) {
                  qreal rx=x+s, ry=y+100;
                  drawPlain(n->right,rx,ry,s/1.8);
              }
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

        // 4) 最终结构
        steps.push_back([this,value](){
          bst.eraseKey(value); // 在 bst.cpp 中已按“前驱替换+右子树接到a->r”规则实现
          view->resetScene();
            view->setTitle("二叉搜索树：删除第4步 接上并完成");
          drawBT(bst.root(),600,120,300,0);
        });

        timer.start();
        return;
    }

    //其它两类（叶子/只有一个孩子）：删边、删点 、真删除
    steps.push_back([this,value](){
      bst.eraseKey(value);
      view->resetScene();
        view->setTitle("二叉搜索树：删除完成");
      drawBT(bst.root(),600,120,300,0);
    });
    timer.start();
}
void MainWindow::bstClear() {
    bst.clear();
    view->resetScene();
    view->setTitle("BST（空）");
}

//哈夫曼树
// —— 新增：哈夫曼树 —— //
void MainWindow::huffmanBuild() {
    // 解析权值
    auto w = parseIntList(huffmanInput->text());
    if (w.isEmpty()) {
        view->resetScene();
        view->setTitle("哈夫曼树：请输入权值序列");
        statusBar()->showMessage("哈夫曼树：无有效输入");
        return;
    }

    // 若已有旧树，释放
    huff.clear();

    // 准备步骤播放
    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 初始森林（叶子结点）；这里直接创建 BTNode 以便最终汇成一棵树交给 huff 管理
    QVector<ds::BTNode*> forest;
    forest.reserve(w.size());
    for (int x : w) {
        forest.push_back(ds::Huffman::makeNode(x));
    }

    auto drawForest = [this](const QVector<ds::BTNode*>& F, const QString& title){
        view->resetScene();
        view->setTitle(title);
        qreal x = 150;
        for (int i = 0; i < F.size(); ++i) {
            drawBT(F[i], x, 120, 60, -99999);
            x += 180;
        }
    };

    // 第 0 步：显示初始森林
    QVector<ds::BTNode*> F0 = forest;
    steps.push_back([this, F0, drawForest](){
        drawForest(F0, QString("哈夫曼树：初始森林（%1 棵）").arg(F0.size()));
        statusBar()->showMessage("哈夫曼树：开始构建");
    });

    // 逐步“选两最小+合并”
    QVector<ds::BTNode*> cur = forest;
    while (cur.size() > 1) {
        int i1 = -1, i2 = -1;
        for (int i = 0; i < cur.size(); i++)
            if (i1 == -1 || cur[i]->key < cur[i1]->key) i1 = i;
        for (int i = 0; i < cur.size(); i++) {
            if (i == i1) continue;
            if (i2 == -1 || cur[i]->key < cur[i2]->key) i2 = i;
        }
        if (i1 > i2) { int t = i1; i1 = i2; i2 = t; }

        int a = cur[i1]->key, b = cur[i2]->key;
        ds::BTNode* parent = ds::Huffman::makeNode(a + b);
        parent->left = cur[i1];
        parent->right = cur[i2];

        QVector<ds::BTNode*> before = cur;
        QVector<ds::BTNode*> after  = cur;
        after[i1] = parent;
        after.remove(i2);

        steps.push_back([this, before, a, b, drawForest](){
            drawForest(before, QString("哈夫曼树：选择最小的两棵：%1 与 %2").arg(a).arg(b));
        });
        steps.push_back([this, after, a, b, parent, drawForest](){
            drawForest(after, QString("哈夫曼树：合并 %1 + %2 -> %3").arg(a).arg(b).arg(parent->key));
        });

        cur[i1] = parent;
        cur.remove(i2);
    }

    // 最终根交给 huff 管理，便于 clear()
    if (!cur.isEmpty())
        huff.rootNode = cur[0];

    steps.push_back([this](){
        view->resetScene();
        view->setTitle("哈夫曼树：构建完成");
        drawBT(huff.root(), 600, 120, 300, -99999);
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





//绘制
void MainWindow::drawSeqlist(const ds::Seqlist& sl){
    view->resetScene();
    view->setTitle("顺序表");
    const int n = sl.size();
    const qreal y = 220;
    for(int i = 0;i < n;i++) {
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
        if(lastx > 0) view->addEdge(QPointF(lastx,y), QPointF(x-34,y));
        lastx = x+34;
        x += 120;
        p = p->next;
        i++;
        idx++;
    }
}
void MainWindow::drawStack(const ds::Stack& st){
    view->resetScene();
    view->setTitle("顺序栈");
    const int n = st.size();
    qreal x=220, y=420;
    for(int i = 0;i < n;i++) {
        view->addNode(x, y - i*90, QString::number(i==n-1? st.getPeek(): st.get(i)));
    }
}
void MainWindow::drawBT(ds::BTNode* root, qreal x, qreal y, qreal distance, int highlightKey){
    if(!root)
        return;
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

