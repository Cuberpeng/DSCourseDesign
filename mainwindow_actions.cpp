#include "mainwindow.h"
#include <QGraphicsScene>
#include <QApplication>
#include <QMessageBox>
#include <QStyle>
#include <QIcon>
#include <QStatusBar>
#include <QTimeLine>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <cmath>
#include <memory>
#include <climits>
#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QSet>

static inline qreal lerp(qreal a, qreal b, qreal t){ return a + (b - a) * t; }

// ===== 顺序表 =====
void MainWindow::seqlistBuild(){
    seq.clear();
    auto a = parseIntList(seqlistInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene();
    view->setTitle(QStringLiteral("顺序表：建立"));
    steps.push_back([this](){ drawSeqlist(seq); showMessage(QStringLiteral("顺序表：开始建立")); });
    for (int x : a){
        steps.push_back([this, x](){ seq.insert(seq.size(), x); drawSeqlist(seq); statusBar()->showMessage(QStringLiteral("顺序表：插入 %1").arg(x)); });
    }
    timer.start();
}

void MainWindow::seqlistInsert(){
    int pos = seqlistPosition->value();
    bool ok = false; int val = seqlistValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("顺序表：请输入有效的值")); return; }

    const int n = seq.size();
    if (pos < 0) pos = 0;
    if (pos > n) pos = n;

    QVector<QString> arr(n);
    for (int i=0;i<n;++i) arr[i] = QString::number(seq.get(i));

    const qreal cellW = 68, cellH = 54, gap = 14;
    const qreal startX = 80, startY = 180;
    const int prevInterval = timer.interval();
    int frames = 10; if (n - pos > 300) frames = 6; if (n - pos > 800) frames = 4;
    const int animInterval = 60;

    timer.stop(); steps.clear(); stepIndex = 0;

    // 步骤1：显示当前状态，高亮插入位置
    steps.push_back([=](){
        view->resetScene(); view->setTitle(QStringLiteral("顺序表：插入前（pos=%1）").arg(pos));
        for (int i=0;i<n;++i){
            qreal x = startX + i*(cellW+gap), y = startY;
            view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#e8eef9")));
            auto* tItem = view->Scene()->addText(arr[i]); tItem->setDefaultTextColor(Qt::black);
            auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
            auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x+cellW/2-6, y+cellH+6);
        }
        // 高亮插入位置
        qreal highlightX = startX + pos*(cellW+gap);
        view->Scene()->addRect(QRectF(highlightX,startY,cellW,cellH), QPen(QColor("#ef4444"),3), QBrush(Qt::transparent));
        showMessage(QStringLiteral("顺序表：准备在位置 %1 插入").arg(pos));
    });

    // 步骤2：移动插入位置之后的元素
    for (int f = 0; f <= frames; ++f) {
        const double t  = double(f) / frames;
        const double dx = t * (cellW + gap);
        steps.push_back([=](){
            if (f == 0) timer.setInterval(animInterval);
            view->resetScene(); view->setTitle(QStringLiteral("顺序表：插入 Step 1（移动元素）"));

            // 绘制插入位置之前的元素（不移动）
            for (int i=0;i<pos;++i){
                qreal x = startX + i*(cellW+gap), y = startY;
                view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#e8eef9")));
                auto* tItem = view->Scene()->addText(arr[i]); tItem->setDefaultTextColor(Qt::black);
                auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
                auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(x+cellW/2-6, y+cellH+6);
            }

            // 绘制正在移动的元素（插入位置及之后的元素）
            for (int i=pos;i<n;++i){
                qreal sx = startX + i*(cellW+gap) + dx;
                qreal sy = startY;
                view->Scene()->addRect(QRectF(sx,sy,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#ffd166")));
                auto* moving = view->Scene()->addText(arr[i]); moving->setDefaultTextColor(Qt::black);
                auto r=moving->boundingRect(); moving->setPos(sx+(cellW-r.width())/2, sy+(cellH-r.height())/2-1);

                // 索引也移动
                auto* idx = view->Scene()->addText(QString::number(i+1)); // 索引+1
                idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(sx+cellW/2-6, sy+cellH+6);
            }

            // 绘制插入位置的新元素（从上方进入）
            qreal newY = startY - 80 * (1-t); // 从上方进入
            qreal newX = startX + pos*(cellW+gap);
            view->Scene()->addRect(QRectF(newX,newY,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#22c55e")));
            auto* newItem = view->Scene()->addText(QString::number(val)); newItem->setDefaultTextColor(Qt::black);
            auto r=newItem->boundingRect(); newItem->setPos(newX+(cellW-r.width())/2, newY+(cellH-r.height())/2-1);
            auto* newIdx = view->Scene()->addText(QString::number(pos)); newIdx->setDefaultTextColor(Qt::darkGray);
            newIdx->setPos(newX+cellW/2-6, newY+cellH+6);

            //showMessage(QStringLiteral("顺序表：移动元素并插入新值"));
            if (f==frames) timer.setInterval(prevInterval);
        });
    }

    // 步骤3：显示最终结果
    steps.push_back([=](){
        seq.insert(pos, val);
        view->resetScene(); view->setTitle(QStringLiteral("顺序表：插入完成（pos=%1, val=%2）").arg(pos).arg(val));

        const int m = n + 1;
        for (int i=0;i<m;++i){
            bool hi = (i==pos);
            qreal x = startX + i*(cellW+gap), y = startY;
            view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(hi? QColor("#22c55e"): QColor("#e8eef9")));

            QString s;
            if (i < pos) s = arr[i];
            else if (i == pos) s = QString::number(val);
            else if (i > pos) s = arr[i-1];

            auto* tItem = view->Scene()->addText(s); tItem->setDefaultTextColor(Qt::black);
            auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
            auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x+cellW/2-6, y+cellH+6);
        }
        showMessage(QStringLiteral("顺序表：插入完成"));
    });

    timer.start();
}

void MainWindow::seqlistErase(){
    int pos = seqlistPosition->value();
    const int n = seq.size();
    if (pos < 0 || pos >= n) { showMessage(QStringLiteral("顺序表：删除失败(位置越界)")); return; }

    QVector<QString> arr(n);
    for (int i=0;i<n;++i) arr[i] = QString::number(seq.get(i));

    const qreal cellW = 68, cellH = 54, gap = 14;
    const qreal startX = 80, startY = 180;
    const int prevInterval = timer.interval();
    int frames = 10; if (n - 1 - pos > 300) frames = 6; if (n - 1 - pos > 800) frames = 4;
    const int animInterval = 60;

    timer.stop(); steps.clear(); stepIndex=0;

    // 步骤1：显示当前状态，高亮要删除的位置
    steps.push_back([=](){
        timer.setInterval(animInterval);
        view->resetScene(); view->setTitle(QStringLiteral("顺序表：删除前（pos=%1）").arg(pos));
        for (int i=0;i<n;++i){
            bool hi = (i==pos);
            qreal x = startX + i*(cellW+gap), y = startY;
            view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(hi? QColor("#ef4444"): QColor("#e8eef9")));
            auto* t = view->Scene()->addText(arr[i]); t->setDefaultTextColor(Qt::black);
            auto r=t->boundingRect(); t->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
            auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x+cellW/2-6, y+cellH+6);
        }
        showMessage(QStringLiteral("顺序表：准备删除位置 %1 的元素").arg(pos));
    });

    // 步骤2：删除元素并移动后续元素
    for (int f = 0; f <= frames; ++f) {
        const double t  = double(f) / frames;
        const double dx = -t * (cellW + gap);
        const bool last = (f==frames);
        steps.push_back([=](){
            view->resetScene(); view->setTitle(QStringLiteral("顺序表：删除 Step 2（移动元素）"));

            // 绘制删除位置之前的元素（不移动）
            for (int i=0;i<pos;++i){
                qreal x = startX + i*(cellW+gap), y = startY;
                view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#e8eef9")));
                auto* tItem = view->Scene()->addText(arr[i]); tItem->setDefaultTextColor(Qt::black);
                auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
                auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(x+cellW/2-6, y+cellH+6);
            }

            // 绘制被删除的元素（逐渐消失）
            if (!last) {
                qreal deleteX = startX + pos*(cellW+gap);
                qreal deleteY = startY - 80 * t; // 向上移动并消失
                qreal alpha = 1.0 - t; // 透明度逐渐降低

                auto* rect = view->Scene()->addRect(QRectF(deleteX,deleteY,cellW,cellH),
                                                   QPen(QColor("#ef4444"),2), QBrush(QColor("#ef4444")));
                rect->setOpacity(alpha);

                auto* text = view->Scene()->addText(arr[pos]); text->setDefaultTextColor(Qt::black);
                text->setOpacity(alpha);
                auto r=text->boundingRect(); text->setPos(deleteX+(cellW-r.width())/2, deleteY+(cellH-r.height())/2-1);

                auto* idx = view->Scene()->addText(QString::number(pos)); idx->setDefaultTextColor(Qt::darkGray);
                idx->setOpacity(alpha);
                idx->setPos(deleteX+cellW/2-6, deleteY+cellH+6);
            }

            // 绘制正在移动的元素（删除位置之后的元素）
            for (int i=pos+1;i<n;++i){
                qreal sx = startX + i*(cellW+gap) + dx;
                qreal sy = startY;
                view->Scene()->addRect(QRectF(sx,sy,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#ffd166")));
                auto* moving = view->Scene()->addText(arr[i]); moving->setDefaultTextColor(Qt::black);
                auto r=moving->boundingRect(); moving->setPos(sx+(cellW-r.width())/2, sy+(cellH-r.height())/2-1);

                // 索引也移动
                auto* idx = view->Scene()->addText(QString::number(i-1)); // 索引-1
                idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(sx+cellW/2-6, sy+cellH+6);
            }

            //showMessage(QStringLiteral("顺序表：删除元素并移动后续元素"));
            if (f==frames) timer.setInterval(prevInterval);
        });
    }

    // 步骤3：显示最终结果
    steps.push_back([=](){
        seq.erase(pos);
        view->resetScene(); view->setTitle(QStringLiteral("顺序表：删除完成（pos=%1）").arg(pos));

        const int m = n - 1;
        for (int i=0;i<m;++i){
            bool hi = (i>=pos && i<n-1); // 高亮被移动过的元素
            qreal x = startX + i*(cellW+gap), y = startY;
            view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(hi? QColor("#ffd166"): QColor("#e8eef9")));

            QString s;
            if (i < pos) s = arr[i];
            else s = arr[i+1];

            auto* tItem = view->Scene()->addText(s); tItem->setDefaultTextColor(Qt::black);
            auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
            auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x+cellW/2-6, y+cellH+6);
        }
        showMessage(QStringLiteral("顺序表：删除完成"));
    });

    timer.start();
}

void MainWindow::seqlistClear() {
    seq.clear(); drawSeqlist(seq); showMessage(QStringLiteral("顺序表：已清空"));
}

// ===== 链表 =====
void MainWindow::linklistBuild(){
    link.clear();
    auto a = parseIntList(linklistInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene(); view->setTitle(QStringLiteral("单链表：建立"));
    steps.push_back([this](){ drawLinklist(link); statusBar()->showMessage(QStringLiteral("单链表：开始建立")); });
    for (int x : a){
        steps.push_back([this, x](){ link.insert(link.size(), x); drawLinklist(link); statusBar()->showMessage(QStringLiteral("单链表：插入 %1").arg(x)); });
    }
    timer.start();
}

// ===== 链表 =====
void MainWindow::linklistInsert() {
    int pos = linklistPosition->value();
    bool ok = false; int v = linklistValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("链表：请输入有效的值")); return; }

    int n = link.size();
    if (pos < 0) pos = 0; if (pos > n) pos = n;

    QVector<int> vals; vals.reserve(n);
    for (int i = 0; i < n; ++i) vals.push_back(link.get(i));

    const qreal y = 220, dx = 120, startX = 150; // 增加startX，让链表整体右移
    QVector<QPointF> centers; centers.reserve(n);
    for (int i = 0; i < n; ++i) centers.push_back(QPointF(startX + i*dx, y));

    const int prevIndex = pos - 1;
    const int succIndex = (pos < n) ? pos : -1;

    timer.stop(); steps.clear(); stepIndex = 0;

    // 步骤1：显示当前链表状态，高亮相关节点
    steps.push_back([=]() {
        view->resetScene(); view->setTitle(QStringLiteral("单链表：插入前"));

        // 绘制头指针
        auto* headLabel = view->Scene()->addText("head");
        headLabel->setDefaultTextColor(QColor("#334155"));
        headLabel->setFont(QFont("Arial", 12, QFont::Bold));
        headLabel->setPos(30, y-10);

        if (n > 0) {
            view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));
        }

        // 绘制所有节点
        for (int i = 0; i < vals.size(); ++i) {
            bool hl = (i == prevIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);

            // 绘制连接线
            if (i > 0) {
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }

        // 显示指针变量 p
        if (prevIndex >= 0) {
            auto* pLabel = view->Scene()->addText("p");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(centers[prevIndex].x()-20, centers[prevIndex].y()-70);

            // 显示 p->next 指针
            auto* pNextLabel = view->Scene()->addText("p->next");
            pNextLabel->setDefaultTextColor(QColor("#3b82f6"));
            pNextLabel->setFont(QFont("Arial", 10, QFont::Bold));
            pNextLabel->setPos(centers[prevIndex].x()+40, centers[prevIndex].y()-50);
        } else if (pos == 0) {
            // 头插法：p 就是 head
            auto* pLabel = view->Scene()->addText("p (head)");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(30, y-50); // 从60改为30
        }

        // 绘制尾指针
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
        tailLabel->setPos(centers[n-1].x()+95, y-10);
        if (n > 0) {
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y));
        }

        showMessage(QStringLiteral("步骤1：定位插入位置的前驱节点 p"));
    });

    // 步骤2：创建新节点 q，在插入位置上方创建
    QPointF finalPos(startX + pos*dx, y);
    QPointF qPos(finalPos.x(), y - 100); // 在最终位置的上方创建，而不是固定位置

    steps.push_back([=]() {
        view->resetScene(); view->setTitle(QStringLiteral("单链表：创建新节点 q"));

        // 绘制原有链表
        for (int i = 0; i < vals.size(); ++i) {
            bool hl = (i == prevIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);

            if (i > 0) {
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }

        // 绘制头指针 - 向左移动
        auto* headLabel = view->Scene()->addText("head");
        headLabel->setDefaultTextColor(QColor("#334155"));
        headLabel->setFont(QFont("Arial", 12, QFont::Bold));
        headLabel->setPos(30, y-20); // 从60改为30
        if (n > 0) view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y)); // 从90改为60

        // 显示指针变量 p
        if (prevIndex >= 0) {
            auto* pLabel = view->Scene()->addText("p");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(centers[prevIndex].x()-20, centers[prevIndex].y()-70);
        } else if (pos == 0) {
            auto* pLabel = view->Scene()->addText("p (head)");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(30, y-50); // 从60改为30
        }

        // 创建新节点 q - 在插入位置上方
        view->addNode(qPos.x(), qPos.y(), QString::number(v), true);
        auto* qLabel = view->Scene()->addText("q");
        qLabel->setDefaultTextColor(QColor("#22c55e"));
        qLabel->setFont(QFont("Arial", 14, QFont::Bold));
        qLabel->setPos(qPos.x()-20, qPos.y()-70);

        // 显示 q->next 为 NULL
        auto* qNextLabel = view->Scene()->addText("q->next = NULL");
        qNextLabel->setDefaultTextColor(QColor("#64748b"));
        qNextLabel->setFont(QFont("Arial", 10, QFont::Bold));
        qNextLabel->setPos(qPos.x()+50, qPos.y()-20);

        // 绘制尾指针 - 向左移动
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
        tailLabel->setPos(centers[n-1].x()+95, y-10); // 从60改为30，向左移动
        if (n > 0) {
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y)); // 从90改为60
        }

        showMessage(QStringLiteral("步骤2：创建新节点"));
    });

    // 步骤3：执行 q->next = p->next
    steps.push_back([=]() {
        view->resetScene(); view->setTitle(QStringLiteral("单链表：执行 q->next = p->next"));

        // 绘制原有链表
        for (int i = 0; i < vals.size(); ++i) {
            bool hl = (i == prevIndex || (succIndex != -1 && i == succIndex));
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);

            if (i > 0) {
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }

        // 绘制头指针 - 向左移动
        auto* headLabel = view->Scene()->addText("head");
        headLabel->setDefaultTextColor(QColor("#334155"));
        headLabel->setFont(QFont("Arial", 12, QFont::Bold));
        headLabel->setPos(30, y-20);
        if (n > 0) view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));

        // 显示指针变量 p
        if (prevIndex >= 0) {
            auto* pLabel = view->Scene()->addText("p");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(centers[prevIndex].x()-20, centers[prevIndex].y()-70);

            // 显示 p->next 指针
            auto* pNextLabel = view->Scene()->addText("p->next");
            pNextLabel->setDefaultTextColor(QColor("#3b82f6"));
            pNextLabel->setFont(QFont("Arial", 10, QFont::Bold));
            pNextLabel->setPos(centers[prevIndex].x()+40, centers[prevIndex].y()-50);
        } else if (pos == 0) {
            auto* pLabel = view->Scene()->addText("p (head)");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(30, y-50);
        }

        // 新节点 q
        view->addNode(qPos.x(), qPos.y(), QString::number(v), true);
        auto* qLabel = view->Scene()->addText("q");
        qLabel->setDefaultTextColor(QColor("#22c55e"));
        qLabel->setFont(QFont("Arial", 14, QFont::Bold));
        qLabel->setPos(qPos.x()-20, qPos.y()-70);

        // 绘制 q->next = p->next 的连接 - 使用更直的箭头
        if (succIndex != -1) {
            // 使用直线连接，避免过度弯曲
            QPointF start(qPos.x()+34, qPos.y());
            QPointF end(centers[succIndex].x()-34, y);

            // 计算控制点，使曲线更平缓
            qreal controlOffset = 40; // 较小的控制点偏移
            QPointF c1(start.x() + controlOffset, start.y());
            QPointF c2(end.x() - controlOffset, end.y());

            view->addCurveArrow(start, c1, c2, end);

            auto* nextLabel = view->Scene()->addText("q->next");
            nextLabel->setDefaultTextColor(QColor("#3b82f6"));
            nextLabel->setFont(QFont("Arial", 10, QFont::Bold));
            nextLabel->setPos((start.x()+end.x())/2 - 25, (start.y()+end.y())/2 - 40); // 调整标签位置
        } else {
            // q->next = nullptr
            auto* nullLabel = view->Scene()->addText("q->next = NULL");
            nullLabel->setDefaultTextColor(QColor("#64748b"));
            nullLabel->setFont(QFont("Arial", 10, QFont::Bold));
            nullLabel->setPos(qPos.x()+50, qPos.y()-20);
        }
        // 绘制尾指针 - 向左移动
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
        tailLabel->setPos(centers[n-1].x()+95, y-10); // 从60改为30，向左移动
        if (n > 0) {
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y)); // 从90改为60
        }

        showMessage(QStringLiteral("步骤3：执行 q->next = p->next"));
    });

    // 步骤4：执行 p->next = q
    steps.push_back([=]() {
    view->resetScene(); view->setTitle(QStringLiteral("单链表：执行 p->next = q"));

    // 绘制原有链表（断开 p 到原后继的连接）
    for (int i = 0; i < vals.size(); ++i) {
        bool hl = (i == prevIndex);
        QPointF c = centers[i];
        view->addNode(c.x(), y, QString::number(vals[i]), hl);
        auto* idxItem = view->Scene()->addText(QString::number(i));
        idxItem->setDefaultTextColor(Qt::darkGray);
        idxItem->setPos(c.x()-6, y+40);

        // 不绘制 p 到原后继的连接
        if (i > 0 && i-1 != prevIndex) {
            view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
        }
    }

    // 绘制头指针 - 向左移动
    auto* headLabel = view->Scene()->addText("head");
    headLabel->setDefaultTextColor(QColor("#334155"));
    headLabel->setFont(QFont("Arial", 12, QFont::Bold));
    headLabel->setPos(30, y-20);

    // 头指针连接
    if (prevIndex >= 0) {
        view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));
    }

    // 显示指针变量 p
    if (prevIndex >= 0) {
        auto* pLabel = view->Scene()->addText("p");
        pLabel->setDefaultTextColor(QColor("#ef4444"));
        pLabel->setFont(QFont("Arial", 14, QFont::Bold));
        pLabel->setPos(centers[prevIndex].x()-20, centers[prevIndex].y()-70);
    } else if (pos == 0) {
        auto* pLabel = view->Scene()->addText("p (head)");
        pLabel->setDefaultTextColor(QColor("#ef4444"));
        pLabel->setFont(QFont("Arial", 14, QFont::Bold));
        pLabel->setPos(30, y-50);
    }

    // 新节点 q
    view->addNode(qPos.x(), qPos.y(), QString::number(v), true);
    auto* qLabel = view->Scene()->addText("q");
    qLabel->setDefaultTextColor(QColor("#22c55e"));
    qLabel->setFont(QFont("Arial", 14, QFont::Bold));
    qLabel->setPos(qPos.x()-20, qPos.y()-70);

    // 绘制 p->next = q 的连接 - 使用更直的箭头
    if (prevIndex >= 0) {
        // p 指向 q
        QPointF start(centers[prevIndex].x()+34, y);
        QPointF end(qPos.x()-34, qPos.y());

        // 计算控制点，使曲线更平缓
        qreal controlOffset = 40;
        QPointF c1(start.x() + controlOffset, start.y());
        QPointF c2(end.x() - controlOffset, end.y());

        view->addCurveArrow(start, c1, c2, end);

        auto* nextLabel = view->Scene()->addText("p->next");
        nextLabel->setDefaultTextColor(QColor("#3b82f6"));
        nextLabel->setFont(QFont("Arial", 10, QFont::Bold));
        nextLabel->setPos((start.x()+end.x())/2 - 25, (start.y()+end.y())/2 - 40); // 调整标签位置
    } else {
        // 头插法：head = q
        view->addEdge(QPointF(60, y), QPointF(qPos.x()-34, qPos.y()));

        auto* headLabel = view->Scene()->addText("head");
        headLabel->setDefaultTextColor(QColor("#334155"));
        headLabel->setFont(QFont("Arial", 12, QFont::Bold));
        headLabel->setPos(30, y-20);
    }

    // 绘制 q->next 的连接（如果存在）并保持可见
    if (succIndex != -1) {
        // 保持 q->next 连接可见
        QPointF qNextStart(qPos.x()+34, qPos.y());
        QPointF qNextEnd(centers[succIndex].x()-34, y);
        view->addEdge(qNextStart, qNextEnd);

        // 添加 q->next 标签
        auto* qNextLabel = view->Scene()->addText("q->next");
        qNextLabel->setDefaultTextColor(QColor("#3b82f6"));
        qNextLabel->setFont(QFont("Arial", 10, QFont::Bold));
        qNextLabel->setPos((qNextStart.x()+qNextEnd.x())/2 - 25, (qNextStart.y()+qNextEnd.y())/2 - 20);
    }

    // 绘制尾指针 - 向左移动
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
        tailLabel->setPos(centers[n-1].x()+95, y-10); // 从60改为30，向左移动
        if (n > 0) {
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y)); // 从90改为60
        }

    showMessage(QStringLiteral("步骤4：执行 p->next = q"));
});

    // 步骤5：调整布局，新节点从上方移动到最终位置
    const int moveFrames = 20;
    const int originalInterval = timer.interval();
    const int fastInterval = 30;

    // 在步骤5开始前设置快速动画
    steps.push_back([=]() {
        timer.setInterval(fastInterval);
        showMessage(QStringLiteral("快速调整节点布局..."));
    });

    for (int f = 0; f <= moveFrames; ++f) {
        steps.push_back([=]() {
            qreal t = (qreal)f / moveFrames;
            // 使用缓动函数使动画更自然
            qreal easedT = 1 - std::pow(1 - t, 2); // 缓出效果

            QPointF currentPos = qPos + (finalPos - qPos) * easedT;

            view->resetScene();
            view->setTitle(QStringLiteral("单链表：调整布局"));

            // 绘制头指针 - 向左移动
            auto* headLabel = view->Scene()->addText("head");
            headLabel->setDefaultTextColor(QColor("#334155"));
            headLabel->setFont(QFont("Arial", 12, QFont::Bold));
            headLabel->setPos(30, y-20); // 从60改为30

            // 绘制所有节点
            for (int i = 0; i <= vals.size(); ++i) {
                qreal currentX, currentY;
                QString text;

                if (i < pos) {
                    currentX = centers[i].x();
                    currentY = y;
                    text = QString::number(vals[i]);
                } else if (i == pos) {
                    currentX = currentPos.x();
                    currentY = currentPos.y();
                    text = QString::number(v);
                } else {
                    // 后续节点向右移动，使用缓动效果
                    currentX = centers[i-1].x() + dx * easedT;
                    currentY = y;
                    text = QString::number(vals[i-1]);
                }

                bool highlight = (i == pos);
                view->addNode(currentX, currentY, text, highlight);

                auto* idxItem = view->Scene()->addText(QString::number(i));
                idxItem->setDefaultTextColor(Qt::darkGray);
                idxItem->setPos(currentX-6, currentY+40);
            }

            // 绘制所有连接
            for (int i = 1; i <= vals.size(); ++i) {
                QPointF startPoint, endPoint;

                // 计算起始点
                if (i-1 < pos) {
                    startPoint = QPointF(centers[i-1].x()+34, y);
                } else if (i-1 == pos) {
                    startPoint = QPointF(currentPos.x()+34, currentPos.y());
                } else {
                    startPoint = QPointF(centers[i-2].x() + dx * easedT + 34, y);
                }

                // 计算结束点
                if (i < pos) {
                    endPoint = QPointF(centers[i].x()-34, y);
                } else if (i == pos) {
                    endPoint = QPointF(currentPos.x()-34, currentPos.y());
                } else {
                    endPoint = QPointF(centers[i-1].x() + dx * easedT - 34, y);
                }

                view->addEdge(startPoint, endPoint);
            }

            // 绘制头指针连接
            if (pos == 0) {
                view->addEdge(QPointF(60, y), QPointF(currentPos.x()-34, currentPos.y())); // 从90改为60
            } else {
                view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y)); // 从90改为60
            }

            // 绘制尾指针 - 向左移动
            auto* tailLabel = view->Scene()->addText("tail");
            tailLabel->setDefaultTextColor(QColor("#334155"));
            tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
            tailLabel->setPos(centers[n-1].x()+195, y-10); // 从60改为30，向左移动
            if (n > 0) {
                view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+195, y)); // 从90改为60
            }

            // 在最后一帧恢复原始计时器间隔
            if (f == moveFrames) {
                timer.setInterval(originalInterval);
            }
        });
    }

    // 步骤6：显示最终结果
    steps.push_back([=]() {
        link.insert(pos, v);
        drawLinklist(link);
        view->setTitle(QStringLiteral("单链表：插入完成"));
        showMessage(QStringLiteral("链表插入完成：insert(%1,%2)").arg(pos).arg(v));
    });

    timer.start();
}

void MainWindow::linklistErase() {
    int pos = linklistPosition->value();
    int n = link.size();
    if (pos < 0 || pos >= n) { showMessage(QStringLiteral("链表：删除失败(位置越界)")); return; }

    QVector<int> vals; vals.reserve(n);
    for (int i = 0; i < n; ++i) vals.push_back(link.get(i));

    const qreal y = 220, dx = 120, startX = 120;
    QVector<QPointF> centers; centers.reserve(n);
    for (int i = 0; i < n; ++i) centers.push_back(QPointF(startX + i*dx, y));

    const int prevIndex = pos - 1, qIndex = pos, succIndex = (pos + 1 < n) ? pos + 1 : -1;

    timer.stop(); steps.clear(); stepIndex = 0;

    // 步骤1：显示当前状态，高亮相关节点
    steps.push_back([=]() {
        view->resetScene(); view->setTitle(QStringLiteral("单链表：删除前"));

        // 绘制头指针
        auto* headLabel = view->Scene()->addText("head");
        headLabel->setDefaultTextColor(QColor("#334155"));
        headLabel->setFont(QFont("Arial", 12, QFont::Bold));
        headLabel->setPos(30, y-10);

        if (n > 0) {
            view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));
        }

        for (int i = 0; i < vals.size(); ++i) {
            bool hl = (i == prevIndex || i == qIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);
            if (i > 0) {
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }

        // 显示指针变量
        if (prevIndex >= 0) {
            auto* pLabel = view->Scene()->addText("p");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(centers[prevIndex].x()-20, centers[prevIndex].y()-70);
        }

        auto* qLabel = view->Scene()->addText("q");
        qLabel->setDefaultTextColor(QColor("#22c55e"));
        qLabel->setFont(QFont("Arial", 14, QFont::Bold));
        qLabel->setPos(centers[qIndex].x()-20, centers[qIndex].y()-70);

        // 绘制尾指针
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
        tailLabel->setPos(centers[n-1].x()+95, y-10);
        if (n > 0) {
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y));
        }

        showMessage(QStringLiteral("步骤1：定位节点 p 和 q (q = p->next)"));
    });

    // 步骤2：执行 q = p->next（已经完成，主要是展示关系）
    steps.push_back([=]() {
        view->resetScene(); view->setTitle(QStringLiteral("单链表：q = p->next"));
        for (int i = 0; i < vals.size(); ++i) {
            bool hl = (i == prevIndex || i == qIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);
            if (i > 0) {
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }

        // 绘制头指针
        auto* headLabel = view->Scene()->addText("head");
        headLabel->setDefaultTextColor(QColor("#334155"));
        headLabel->setFont(QFont("Arial", 12, QFont::Bold));
        headLabel->setPos(30, y-10);
        if (n > 0) { view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));}

        // 显示指针变量和关系
        if (prevIndex >= 0) {
            auto* pLabel = view->Scene()->addText("p");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(centers[prevIndex].x()-20, centers[prevIndex].y()-70);

            // 显示 p->next 指向 q
            auto* pNextLabel = view->Scene()->addText("p->next");
            pNextLabel->setDefaultTextColor(QColor("#3b82f6"));
            pNextLabel->setFont(QFont("Arial", 10, QFont::Bold));
            pNextLabel->setPos(centers[prevIndex].x()+40, centers[prevIndex].y()-50);
        }

        auto* qLabel = view->Scene()->addText("q");
        qLabel->setDefaultTextColor(QColor("#22c55e"));
        qLabel->setFont(QFont("Arial", 14, QFont::Bold));
        qLabel->setPos(centers[qIndex].x()-20, centers[qIndex].y()-70);

        // 显示 q->next
        if (succIndex != -1) {
            auto* qNextLabel = view->Scene()->addText("q->next");
            qNextLabel->setDefaultTextColor(QColor("#3b82f6"));
            qNextLabel->setFont(QFont("Arial", 10, QFont::Bold));
            qNextLabel->setPos(centers[qIndex].x()+40, centers[qIndex].y()-50);
        }

        // 绘制尾指针
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
        tailLabel->setPos(centers[n-1].x()+95, y-10);
        if (n > 0) {
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y));
        }

        showMessage(QStringLiteral("步骤2：q = p->next"));
    });

    // 步骤3：执行 p->next = q->next
    steps.push_back([=]() {
        view->resetScene(); view->setTitle(QStringLiteral("单链表：p->next = q->next"));

        // 绘制头指针
         auto* headLabel = view->Scene()->addText("head");
         headLabel->setDefaultTextColor(QColor("#334155"));
         headLabel->setFont(QFont("Arial", 12, QFont::Bold));
         headLabel->setPos(30, y-10);
         if (n > 0) { view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));}

        // 绘制所有节点
        for (int i = 0; i < vals.size(); ++i) {
            bool hl = (i == prevIndex || i == qIndex || (succIndex != -1 && i == succIndex));
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);

            // 绘制边，但跳过 p->q 的连接
            if (i > 0 && i-1 != prevIndex) {
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }

        // 头指针连接
        if (prevIndex > 0) {
            view->addEdge(QPointF(90, y), QPointF(centers[0].x()-34, y));
        }

        // 显示指针变量
        if (prevIndex >= 0) {
            auto* pLabel = view->Scene()->addText("p");
            pLabel->setDefaultTextColor(QColor("#ef4444"));
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(centers[prevIndex].x()-20, centers[prevIndex].y()-70);
        }

        auto* qLabel = view->Scene()->addText("q");
        qLabel->setDefaultTextColor(QColor("#22c55e"));
        qLabel->setFont(QFont("Arial", 14, QFont::Bold));
        qLabel->setPos(centers[qIndex].x()-20, centers[qIndex].y()-70);

        // 绘制新的连接 p->next = q->next
        if (prevIndex >= 0 && succIndex != -1) {
            QPointF start(centers[prevIndex].x()+34, y);
            QPointF end(centers[succIndex].x()-34, y);
            QPointF c1(start.x()+60, start.y()-80), c2(end.x()-60, end.y()-80);
            view->addCurveArrow(start, c1, c2, end);

            auto* newNextLabel = view->Scene()->addText("p->next = q->next");
            newNextLabel->setDefaultTextColor(QColor("#3b82f6"));
            newNextLabel->setFont(QFont("Arial", 10, QFont::Bold));
            newNextLabel->setPos((start.x()+end.x())/2 - 50, (start.y()+end.y())/2 - 100);
        } else if (prevIndex >= 0) {
            // p->next = nullptr
            auto* nullLabel = view->Scene()->addText("p->next = NULL");
            nullLabel->setDefaultTextColor(QColor("#64748b"));
            nullLabel->setFont(QFont("Arial", 10, QFont::Bold));
            nullLabel->setPos(centers[prevIndex].x()+50, centers[prevIndex].y()-20);
        } else {
            // 删除头节点：head = q->next
            if (succIndex != -1) {
                view->addEdge(QPointF(90, y), QPointF(centers[succIndex].x()-34, y));
            }
        }

        // 仍然显示 q->next（即将被删除）
        if (succIndex != -1) {
            view->addEdge(QPointF(centers[qIndex].x()+34, y), QPointF(centers[succIndex].x()-34, y));
        }

        // 绘制尾指针
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
        tailLabel->setPos(centers[n-1].x()+95, y-10);
        if (n > 0) {
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y));
        }

        showMessage(QStringLiteral("步骤3：执行 p->next = q->next"));
    });

    // 步骤4：执行 delete q（节点下落消失）
const int deleteFrames = 20;
const int originalInterval = timer.interval();
const int fastInterval = 30;

// 在删除动画开始前设置快速动画
steps.push_back([=]() {
    timer.setInterval(fastInterval);
    showMessage(QStringLiteral("快速删除节点..."));
});

for (int f = 0; f <= deleteFrames; ++f) {
    steps.push_back([=]() {
        const qreal t = qreal(f) / deleteFrames;
        // 使用缓动函数
        qreal easedT = 1 - std::pow(1 - t, 2);

        view->resetScene();
        view->setTitle(QStringLiteral("单链表：delete q"));

        // 绘制头指针
        auto* headLabel = view->Scene()->addText("head");
        headLabel->setDefaultTextColor(QColor("#334155"));
        headLabel->setFont(QFont("Arial", 12, QFont::Bold));
        headLabel->setPos(30, y-10);
        if (n > 0) {
            view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));
        }

        // 记录前驱节点和后继节点的位置
        QPointF prevNodePos, nextNodePos;
        bool hasPrev = false, hasNext = false;

        // 绘制未被删除的节点
        for (int i = 0; i < vals.size(); ++i) {
            if (i == qIndex) continue;

            QPointF c = centers[i];
            bool hl = (i == prevIndex || (succIndex != -1 && i == succIndex));
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i < qIndex ? i : i-1));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);

            // 记录前驱节点和后继节点的位置
            if (i == prevIndex) {
                prevNodePos = QPointF(c.x(), y);
                hasPrev = true;
            }
            if (i == succIndex) {
                nextNodePos = QPointF(c.x(), y);
                hasNext = true;
            }
        }

        // 绘制所有连接 - 确保指针保持连接
        for (int i = 1; i < vals.size(); ++i) {
            if (i == qIndex) continue; // 跳过被删除节点

            int prevI = i-1;
            if (prevI == qIndex) continue; // 如果前一个节点是被删除的节点，跳过

            // 绘制正常的连接
            if (prevI >= 0 && prevI != qIndex && i != qIndex) {
                view->addEdge(QPointF(centers[prevI].x()+34, y),
                            QPointF(centers[i].x()-34, y));
            }
        }

        // 特别绘制前驱节点和后继节点之间的连接（重要修复）
        if (hasPrev && hasNext) {
            view->addEdge(QPointF(prevNodePos.x()+34, prevNodePos.y()),
                        QPointF(nextNodePos.x()-34, nextNodePos.y()));
        } else if (hasPrev && !hasNext) {
            // 如果删除的是尾节点，前驱节点的next指向null
            auto* nullLabel = view->Scene()->addText("p->next = NULL");
            nullLabel->setDefaultTextColor(QColor("#64748b"));
            nullLabel->setFont(QFont("Arial", 10, QFont::Bold));
            nullLabel->setPos(prevNodePos.x()+50, prevNodePos.y()-20);
        } else if (!hasPrev && hasNext) {
            // 如果删除的是头节点，头指针指向后继节点
            view->addEdge(QPointF(60, y), QPointF(nextNodePos.x()-34, y));
        }

        // 绘制下落的被删除节点
        QPointF deletePos(centers[qIndex].x(), centers[qIndex].y() + 150 * easedT);
        qreal opacity = 1.0 - easedT;

        // 使用特殊样式绘制被删除的节点
        auto* node = view->Scene()->addEllipse(QRectF(deletePos.x()-33, deletePos.y()-33, 66, 66),
                                              QPen(QColor("#ef4444"), 3), QBrush(QColor("#fecaca")));
        node->setOpacity(opacity);

        auto* text = view->Scene()->addText(QString::number(vals[qIndex]));
        text->setDefaultTextColor(Qt::black);
        text->setOpacity(opacity);
        QRectF tb = text->boundingRect();
        text->setPos(deletePos.x() - tb.width()/2, deletePos.y() - tb.height()/2);

        auto* qLabel = view->Scene()->addText("q");
        qLabel->setDefaultTextColor(QColor("#22c55e"));
        qLabel->setFont(QFont("Arial", 12, QFont::Bold));
        qLabel->setOpacity(opacity);
        qLabel->setPos(deletePos.x()-15, deletePos.y()-60);

        auto* deleteLabel = view->Scene()->addText("delete q");
        deleteLabel->setDefaultTextColor(QColor("#ef4444"));
        deleteLabel->setFont(QFont("Arial", 10, QFont::Bold));
        deleteLabel->setPos(deletePos.x()-30, deletePos.y()+50);

        // 在最后一帧恢复原始计时器间隔
        if (f == deleteFrames) {
            timer.setInterval(originalInterval);
        }

        // 绘制尾指针
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 12, QFont::Bold));

        // 如果删除的是尾节点，更新尾指针位置
        if (qIndex == n-1 && hasPrev) {
            tailLabel->setPos(prevNodePos.x()+95, y-10);
            view->addEdge(QPointF(prevNodePos.x()+35, y), QPointF(prevNodePos.x()+95, y));
        } else if (qIndex != n-1) {
            tailLabel->setPos(centers[n-1].x()+95, y-10);
            view->addEdge(QPointF(centers[n-1].x()+35, y), QPointF(centers[n-1].x()+95, y));
        } else {
            // 如果链表为空
            tailLabel->setPos(150, y-10);
        }

        showMessage(QStringLiteral("步骤4：执行 delete q"));
    });
}

    // 步骤5：调整布局（压缩链表）
    const int adjustFrames = 20; // 增加帧数

    // 设置快速动画
    steps.push_back([=]() {
        timer.setInterval(fastInterval);
        showMessage(QStringLiteral("快速调整布局..."));
    });

    for (int f = 0; f <= adjustFrames; ++f) {
        steps.push_back([=]() {
            const qreal t = qreal(f) / adjustFrames;
            // 使用缓动函数
            qreal easedT = 1 - std::pow(1 - t, 2);

            view->resetScene(); view->setTitle(QStringLiteral("单链表：调整布局"));

            // 绘制头指针
            auto* headLabel = view->Scene()->addText("head");
            headLabel->setDefaultTextColor(QColor("#334155"));
            headLabel->setFont(QFont("Arial", 12, QFont::Bold));
            headLabel->setPos(30, y-10);
            if (n > 0) { view->addEdge(QPointF(60, y), QPointF(centers[0].x()-34, y));}

            // 计算调整后的位置
            for (int i = 0; i < n; ++i) {
                if (i == qIndex) continue;

                int displayIndex = i < qIndex ? i : i-1;
                qreal currentX, currentY;

                if (i < qIndex) {
                    // 删除位置之前的节点保持原位
                    currentX = centers[i].x();
                    currentY = y;
                } else {
                    // 删除位置之后的节点向左移动
                    currentX = centers[i].x() - dx * easedT;
                    currentY = y;
                }

                bool highlight = (i == prevIndex || i == succIndex);
                view->addNode(currentX, currentY, QString::number(vals[i]), highlight);

                auto* idxItem = view->Scene()->addText(QString::number(displayIndex));
                idxItem->setDefaultTextColor(Qt::darkGray);
                idxItem->setPos(currentX-6, currentY+40);
            }

            // 绘制所有连接 - 确保指针不断开
            for (int i = 1; i < n; ++i) {
                if (i == qIndex) continue;

                int prevI = i-1;
                if (prevI == qIndex) prevI = i-2; // 跳过被删除的节点
                if (prevI < 0) continue;

                qreal prevX, prevY;
                if (prevI < qIndex) {
                    prevX = centers[prevI].x();
                    prevY = y;
                } else {
                    prevX = centers[prevI].x() - dx * easedT;
                    prevY = y;
                }

                qreal currentX, currentY;
                if (i < qIndex) {
                    currentX = centers[i].x();
                    currentY = y;
                } else {
                    currentX = centers[i].x() - dx * easedT;
                    currentY = y;
                }

                view->addEdge(QPointF(prevX+34, y), QPointF(currentX-34, y));
            }

            // 头指针连接
            // if (qIndex > 0) {
            //     view->addEdge(QPointF(90, y), QPointF(centers[0].x()-34, y));
            // } else if (succIndex != -1) {
            //     // 删除头节点，头指针指向新的头节点
            //     qreal newHeadX = centers[succIndex].x() - dx * easedT;
            //     view->addEdge(QPointF(90, y), QPointF(newHeadX-34, y));
            // }

            // 在最后一帧恢复原始计时器间隔
            if (f == adjustFrames) {
                timer.setInterval(originalInterval);
            }
            // 绘制尾指针
            auto* tailLabel = view->Scene()->addText("tail");
            tailLabel->setDefaultTextColor(QColor("#334155"));
            tailLabel->setFont(QFont("Arial", 12, QFont::Bold));
            tailLabel->setPos(centers[n-1].x()+95, y-10);
            if (n > 0) {
                view->addEdge(QPointF(centers[n-2].x()+35, y), QPointF(centers[n-1].x()+95, y));
            }

            showMessage(QStringLiteral("步骤5：调整节点位置"));
        });
    }

    // 步骤6：显示最终结果
    steps.push_back([=]() {
        link.erase(pos);
        drawLinklist(link);
        view->setTitle(QStringLiteral("单链表：删除完成"));
        showMessage(QStringLiteral("链表：删除位置 %1 完成").arg(pos));
    });

    timer.start();
}

void MainWindow::linklistClear() { link.clear(); drawLinklist(link); statusBar()->showMessage(QStringLiteral("链表：已清空")); }

// ===== 栈 =====
void MainWindow::stackBuild(){
    st.clear();
    auto a = parseIntList(stackInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene(); view->setTitle(QStringLiteral("栈：建立"));
    steps.push_back([this](){ drawStack(st); showMessage(QStringLiteral("栈：开始建立")); });
    for (int x : a){
        steps.push_back([this, x](){ st.push(x); drawStack(st); showMessage(QStringLiteral("栈：插入 %1").arg(x)); });
    }
    timer.start();
}

void MainWindow::stackPush() {
    bool ok = false; int v = stackValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("栈：请输入有效的值")); return; }

    const qreal x0 = 380, y0 = 120, W = 300, T = 12, innerPad = 6;
    const qreal BLOCK_H = 32, GAP = 4;
    const int   n = st.size();
    const int   levels = std::max(n, 6);
    const qreal innerH = levels * BLOCK_H + (levels - 1) * GAP;
    const qreal H = innerH + T + BLOCK_H;
    const qreal innerW = W - 2*T - 2*innerPad;
    const qreal leftX  = x0 + T + innerPad;
    const qreal bottomInnerY = y0 + H - T;
    const qreal yTopBlock = bottomInnerY - (n+1) * BLOCK_H - (n) * GAP;
    const qreal xCenter   = leftX + innerW/2;
    const qreal yStart    = yTopBlock - 80;

    const int frames = 10; const int oldInterval = timer.interval();

    timer.stop(); steps.clear(); stepIndex = 0;

    for (int f=0; f<=frames; ++f){
        steps.push_back([=](){
            if (f==0) timer.setInterval(60);
            const qreal t = qreal(f)/frames;
            view->resetScene(); view->setTitle(QStringLiteral("栈：入栈（移动中）"));
            QGraphicsScene* S = view->Scene();
            QBrush wall(QColor("#334155")); QPen none(Qt::NoPen);
            S->addRect(QRectF(x0, y0, T, H), none, wall);
            S->addRect(QRectF(x0+W-T, y0, T, H), none, wall);
            S->addRect(QRectF(x0, y0+H-T, W, T), none, wall);
            QPen boxPen(QColor("#1e293b")); boxPen.setWidthF(1.2);
            QBrush fill(QColor("#93c5fd")), topFill(QColor("#60a5fa"));
            const int nNow = st.size();
            const qreal innerW2 = innerW;
            const qreal leftX2  = leftX;
            for (int i = 0; i < nNow; ++i) {
                const bool isTop = (i == nNow - 1);
                const qreal yTop = bottomInnerY - (i + 1) * BLOCK_H - i * GAP;
                S->addRect(QRectF(leftX2, yTop, innerW2, BLOCK_H), boxPen, isTop ? topFill : fill);
                auto* label = S->addText(QString::number(st.get(i)));
                QRectF tb = label->boundingRect(); label->setDefaultTextColor(Qt::black);
                label->setPos(leftX2 + innerW2/2 - tb.width()/2, yTop + BLOCK_H/2 - tb.height()/2);
            }
            qreal yTop = lerp(yStart, yTopBlock, t);
            S->addRect(QRectF(leftX, yTop, innerW, BLOCK_H), boxPen, topFill);
            auto* label = S->addText(QString::number(v)); label->setDefaultTextColor(Qt::black);
            QRectF tb = label->boundingRect(); label->setPos(xCenter - tb.width()/2, yTop + BLOCK_H/2 - tb.height()/2);
            if (f==frames) timer.setInterval(oldInterval);
        });
    }

    steps.push_back([=](){ st.push(v); drawStack(st); view->setTitle(QStringLiteral("栈：入栈完成")); showMessage(QStringLiteral("栈：push(%1)").arg(v)); });
    timer.start();
}

void MainWindow::stackPop() {
    if(st.size()==0){ showMessage(QStringLiteral("栈：空栈，无法出栈")); return; }

    const qreal x0 = 380, y0 = 120, W = 300, T = 12, innerPad = 6;
    const qreal BLOCK_H = 32, GAP = 4;
    const int   n = st.size();
    const int   levels = std::max(n, 6);
    const qreal innerH = levels * BLOCK_H + (levels - 1) * GAP;
    const qreal H = innerH + T + BLOCK_H;
    const qreal innerW = W - 2*T - 2*innerPad;
    const qreal leftX  = x0 + T + innerPad;
    const qreal bottomInnerY = y0 + H - T;
    const qreal xCenter = leftX + innerW/2;

    int topVal = st.getPeek();
    const qreal yTopBlock = bottomInnerY - n * BLOCK_H - (n - 1) * GAP;
    const qreal yEnd = yTopBlock - 80;

    const int frames = 10; const int oldInterval = timer.interval();

    timer.stop(); steps.clear(); stepIndex = 0;

    for (int f=0; f<=frames; ++f){
        steps.push_back([=](){
            if (f==0) timer.setInterval(60);
            const qreal t = qreal(f)/frames;
            view->resetScene(); view->setTitle(QStringLiteral("栈：出栈（移动中）"));

            QGraphicsScene* S = view->Scene();
            QBrush wall(QColor("#334155")); QPen none(Qt::NoPen);
            S->addRect(QRectF(x0, y0, T, H), none, wall);
            S->addRect(QRectF(x0+W-T, y0, T, H), none, wall);
            S->addRect(QRectF(x0, y0+H-T, W, T), none, wall);

            QPen boxPen(QColor("#1e293b")); boxPen.setWidthF(1.2);
            QBrush fill(QColor("#93c5fd")), topFill(QColor("#60a5fa"));
            for (int i = 0; i < n-1; ++i) {
                const bool isTop = (i == n - 2);
                const qreal yTop = bottomInnerY - (i + 1) * BLOCK_H - i * GAP;
                S->addRect(QRectF(leftX, yTop, innerW, BLOCK_H), boxPen, isTop ? topFill : fill);
                auto* label = S->addText(QString::number(st.get(i)));
                QRectF tb = label->boundingRect(); label->setDefaultTextColor(Qt::black);
                label->setPos(leftX + innerW/2 - tb.width()/2, yTop + BLOCK_H/2 - tb.height()/2);
            }
            qreal yTop = lerp(yTopBlock, yEnd, t);
            S->addRect(QRectF(leftX, yTop, innerW, BLOCK_H), boxPen, topFill);
            auto* label = S->addText(QString::number(topVal)); label->setDefaultTextColor(Qt::black);
            QRectF tb = label->boundingRect(); label->setPos(xCenter - tb.width()/2, yTop + BLOCK_H/2 - tb.height()/2);

            if (f==frames) timer.setInterval(oldInterval);
        });
    }

    steps.push_back([this](){ int out=0; st.pop(&out); drawStack(st); view->setTitle(QStringLiteral("栈：出栈完成")); showMessage(QStringLiteral("栈：%1 出栈").arg(out)); });
    timer.start();
}

void MainWindow::stackClear() { st.clear(); drawStack(st); showMessage(QStringLiteral("栈：已清空")); }

// ===== 二叉树（构建 & 遍历逐帧高亮） =====
void MainWindow::btBuild(){
    auto a = parseIntList(btInput->text());
    int sent = btNull->value();
    timer.stop(); steps.clear(); stepIndex = 0;

    steps.push_back([this](){
        bt.clear(); view->resetScene(); view->setTitle(QStringLiteral("二叉树：开始建立（空树）"));
        drawBT(bt.root(), 400, 120, 200, 0); showMessage(QStringLiteral("二叉树：开始建立（空树）"));
    });

    for (int i = 0; i < a.size(); ++i){
        steps.push_back([=](){
            QVector<int> b(a.size(), sent);
            for (int k = 0; k <= i; k++) b[k] = a[k];
            bt.clear(); bt.buildTree(b.data(), b.size(), sent);
            view->resetScene();
            QString msg = (a[i]==sent) ? QStringLiteral("空位(哨兵 %1) 跳过").arg(sent) : QStringLiteral("插入 %1").arg(a[i]);
            view->setTitle(QStringLiteral("二叉树：%1 / 共 %2 步").arg(msg).arg(a.size()));
            drawBT(bt.root(), 400, 120, 200, 0);
            showMessage(QStringLiteral("二叉树：步骤 %1/%2，%3").arg(i+1).arg(a.size()).arg(msg));
        });
    }
    timer.start();
}

void MainWindow::btClear() {
    bt.clear(); view->resetScene(); view->setTitle(QStringLiteral("二叉树（空）")); showMessage(QStringLiteral("二叉树：已清空"));
}

void MainWindow::btPreorder() {
    int need = bt.preorder(nullptr, 0);
    if (need <= 0) { view->resetScene(); view->setTitle(QStringLiteral("先序遍历：空树")); drawBT(bt.root(), 400, 120, 200, 0); return; }
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.preorder(buf.get(), need);

    timer.stop(); steps.clear(); stepIndex = 0;
    for (int i=0;i<n;++i){
        int key = buf[i];
        steps.push_back([=](){ view->resetScene(); view->setTitle(QStringLiteral("先序遍历：访问 %1（%2/%3）").arg(key).arg(i+1).arg(n)); drawBT(bt.root(), 400, 120, 200, key); });
    }
    timer.start();
}
void MainWindow::btInorder() {
    int need = bt.inorder(nullptr, 0);
    if (need <= 0) { view->resetScene(); view->setTitle(QStringLiteral("中序遍历：空树")); drawBT(bt.root(), 400, 120, 200, 0); return; }
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.inorder(buf.get(), need);

    timer.stop(); steps.clear(); stepIndex = 0;
    for (int i=0;i<n;++i){
        int key = buf[i];
        steps.push_back([=](){ view->resetScene(); view->setTitle(QStringLiteral("中序遍历：访问 %1（%2/%3）").arg(key).arg(i+1).arg(n)); drawBT(bt.root(), 400, 120, 200, key); });
    }
    timer.start();
}
void MainWindow::btPostorder() {
    int need = bt.postorder(nullptr, 0);
    if (need <= 0) { view->resetScene(); view->setTitle(QStringLiteral("后序遍历：空树")); drawBT(bt.root(), 400, 120, 200, 0); return; }
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.postorder(buf.get(), need);

    timer.stop(); steps.clear(); stepIndex = 0;
    for (int i=0;i<n;++i){
        int key = buf[i];
        steps.push_back([=](){ view->resetScene(); view->setTitle(QStringLiteral("后序遍历：访问 %1（%2/%3）").arg(key).arg(i+1).arg(n)); drawBT(bt.root(), 400, 120, 200, key); });
    }
    timer.start();
}
void MainWindow::btLevelorder() {
    // 先检查树是否为空
    if (bt.root() == nullptr) {
        view->resetScene();
        view->setTitle(QStringLiteral("层序遍历：空树"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("层序遍历：空树"));
        return;
    }

    // 获取节点数量
    int need = bt.levelorder(nullptr, 0);
    if (need <= 0) {
        view->resetScene();
        view->setTitle(QStringLiteral("层序遍历：空树"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("层序遍历：空树"));
        return;
    }

    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.levelorder(buf.get(), need);

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 添加初始状态
    steps.push_back([=](){
        view->resetScene();
        view->setTitle(QStringLiteral("层序遍历：开始"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("层序遍历：开始"));
    });

    for (int i=0;i<n;++i){
        int key = buf[i];
        steps.push_back([=](){
            view->resetScene();
            view->setTitle(QStringLiteral("层序遍历：访问 %1（%2/%3）").arg(key).arg(i+1).arg(n));
            drawBT(bt.root(), 400, 120, 200, key);
            showMessage(QStringLiteral("层序遍历：访问 %1").arg(key));
        });
    }

    // 添加结束状态
    steps.push_back([=](){
        view->resetScene();
        view->setTitle(QStringLiteral("层序遍历：完成"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("层序遍历：完成"));
    });

    timer.start();
}

// ===== 二叉搜索树 =====
void MainWindow::bstBuild() {
    auto a = parseIntList(bstInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    steps.push_back([this]() { bst.clear(); view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：开始构建")); drawBT(bst.root(), 400, 120, 200, 0); });
    for (int i = 0; i < a.size(); i++) {
        steps.push_back([=]() { bst.insert(a[i]); view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：插入 %1（第 %2/%3 步）").arg(a[i]).arg(i+1).arg(a.size())); drawBT(bst.root(), 400, 120, 200, 0); });
    }
    timer.start();
}

void MainWindow::bstFind() {
    bool ok = false; int value = bstValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("二叉搜索树：请输入有效的键值")); return; }

    QVector<int> path; ds::BTNode* p = bst.root();
    while(p){ path.push_back(p->key); if (value < p->key) p = p->left; else if (value > p->key) p = p->right; else break; }
    bool found = (!path.isEmpty() && path.last() == value);

    timer.stop(); steps.clear(); stepIndex = 0;
    for (int i=0;i<path.size();++i){
        int key = path[i]; const bool last = (i==path.size()-1);
        steps.push_back([=](){ view->resetScene();
            view->setTitle(QStringLiteral("BST 查找 %1：%2（%3/%4）").arg(value).arg(last ? (found?QStringLiteral("找到"):QStringLiteral("未找到")) : QStringLiteral("遍历中")).arg(i+1).arg(path.size()));
            drawBT(bst.root(), 400, 120, 200, key); });
    }
    // 添加最后一步：显示结果弹窗
    steps.push_back([=](){
        view->resetScene();
        view->setTitle(QStringLiteral("BST 查找 %1：%2").arg(value).arg(found ? QStringLiteral("找到") : QStringLiteral("未找到")));
        drawBT(bst.root(), 400, 120, 200, found ? value : 0);
        showMessage(QStringLiteral("BST 查找：%1 %2").arg(value).arg(found ? QStringLiteral("查找成功") : QStringLiteral("查找失败")));

        // 显示弹窗
        if (found) {
            QMessageBox::information(this,QStringLiteral("查找成功"),QStringLiteral("成功找到元素 %1").arg(value),QMessageBox::Ok);
        } else {
            QMessageBox::warning(this,QStringLiteral("查找失败"),QStringLiteral("未找到元素 %1，请重试").arg(value),QMessageBox::Ok);
        }
    });
    timer.start();
}

void MainWindow::bstInsert() {
    bool ok = false;
    int value = bstValue->text().toInt(&ok);
    if(!ok) {
        showMessage(QStringLiteral("二叉搜索树：请输入有效的键值"));
        return;
    }

    // 检查值是否已存在
    if (bst.find(value) != nullptr) {
        showMessage(QStringLiteral("二叉搜索树：键值 %1 已存在").arg(value));
        return;
    }

    // 查找插入路径
    QVector<int> path;
    ds::BTNode* p = bst.root();

    // 记录查找路径
    while(p) {
        path.push_back(p->key);
        if (value < p->key) {
            p = p->left;
        } else if (value > p->key) {
            p = p->right;
        } else {
            break; // 不应该发生，因为前面已经检查过
        }
    }

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 步骤1：显示当前树状态，高亮查找路径
    for (int i = 0; i < path.size(); ++i) {
        int key = path[i];
        bool last = (i == path.size() - 1);
        steps.push_back([=]() {
            view->resetScene();
            QString stepDesc = last ? QStringLiteral("找到插入位置") : QStringLiteral("查找路径");
            view->setTitle(QStringLiteral("BST 插入 %1：%2（%3/%4）")
                          .arg(value).arg(stepDesc).arg(i+1).arg(path.size()));
            drawBT(bst.root(), 400, 120, 200, key);
            showMessage(QStringLiteral("BST 插入：正在查找插入位置"));
        });
    }

    // 步骤2：显示新节点插入的动画
    steps.push_back([=]() {
        view->resetScene();
        view->setTitle(QStringLiteral("BST 插入 %1：创建新节点").arg(value));

        // 绘制原树
        drawBT(bst.root(), 400, 120, 200, 0);

        // 在新位置显示新节点（从上方进入的动画效果）
        qreal x = 400, y = 120, distance = 200;

        // 计算新节点的位置（模拟插入过程）
        ds::BTNode* current = bst.root();
        ds::BTNode* parent = nullptr;
        bool isLeft = false;

        while (current) {
            parent = current;
            if (value < current->key) {
                current = current->left;
                isLeft = true;
                x -= distance;
            } else {
                current = current->right;
                isLeft = false;
                x += distance;
            }
            y += 100;
            distance /= 1.8;
        }

        // 绘制新节点（从上方进入）
        qreal newY = y - 100; // 从上方一点的位置开始
        view->addNode(x, newY, QString::number(value), true);

        // 绘制连接线（虚线表示将要连接）
        QPen dashPen(QColor("#3b82f6"));
        dashPen.setStyle(Qt::DashLine);
        dashPen.setWidth(2);

        if (parent) {
            QPointF parentPos(x + (isLeft ? distance * 1.8 : -distance * 1.8), y - 100);
            QPointF childPos(x, newY + 34);
            view->Scene()->addLine(QLineF(parentPos, childPos), dashPen);
        }

        showMessage(QStringLiteral("BST 插入：创建新节点 %1").arg(value));
    });

    // 步骤3：执行实际插入并显示结果
    steps.push_back([=]() {
        bst.insert(value);
        view->resetScene();
        view->setTitle(QStringLiteral("BST 插入 %1：完成").arg(value));
        drawBT(bst.root(), 400, 120, 200, value);
        showMessage(QStringLiteral("BST 插入：%1 插入完成").arg(value));
    });

    timer.start();
}

void MainWindow::bstErase() {
    bool ok = false; int value = bstValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("二叉搜索树：请输入有效的键值")); return; }

    ds::BTNode* parent = nullptr; ds::BTNode* p = bst.root();
    while(p && p->key!=value) { parent = p; p = (value < p->key)? p->left : p->right; }

    timer.stop(); steps.clear(); stepIndex = 0;
    if (!p) {
        steps.push_back([=]() { view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：删除 %1：未找到").arg(value)); drawBT(bst.root(),400,120,200,0); });
        timer.start(); return;
    }

    auto drawPlain = [this](ds::BTNode* r, qreal x, qreal y, qreal distance, int highlight=INT_MIN){
        std::function<void(ds::BTNode*, qreal, qreal, qreal)> f;
        f = [this,highlight,&f](ds::BTNode* n,qreal x,qreal y,qreal s){
            if(!n) return;
            bool hl = (n->key == highlight);
            view->addNode(x,y, QString::number(n->key), hl);
            if(n->left)  { qreal lx = x-s, ly=y+100; view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34)); f(n->left, lx,ly,s/1.8); }
            if(n->right) { qreal rx = x+s, ry=y+100; view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34)); f(n->right,rx,ry,s/1.8); }
        };
        f(r,x,y,distance);
    };

    // 1) 断开与 p 相连边
    steps.push_back([=](){
      std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
      g = [=,&g](ds::BTNode* n,qreal x,qreal y,qreal s){
        if(!n) return;
        view->addNode(x,y, QString::number(n->key));
        if(n->left){  qreal lx=x-s, ly=y+100; if(!(n==p || n->left==p))  view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34)); g(n->left, lx,ly,s/1.8); }
        if(n->right){ qreal rx=x+s, ry=y+100; if(!(n==p || n->right==p)) view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34)); g(n->right,rx,ry,s/1.8); }
      };
      view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：删除第1步 删指针域（与 p 相连的边）")); g(bst.root(),400,120,200);
    });

    // 2) 不画 p，本身的左右子树原位保留
    steps.push_back([=](){
      std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
      g = [=,&g,drawPlain](ds::BTNode* n,qreal x,qreal y,qreal s){
        if(!n) return;
        if(n==p){
          if(n->left)  { qreal lx=x-s, ly=y+100; drawPlain(n->left, lx,ly,s/1.8); }
          if(n->right) { qreal rx=x+s, ry=y+100; drawPlain(n->right,rx,ry,s/1.8); }
          return;
        }
        view->addNode(x,y, QString::number(n->key));
        if(n->left)  { qreal lx=x-s, ly=y+100; view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34)); g(n->left, lx,ly,s/1.8); }
        if(n->right) { qreal rx=x+s, ry=y+100; view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34)); g(n->right,rx,ry,s/1.8); }
      };
      view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：删除第2步 删值域（移除 p 本身）")); g(bst.root(),400,120,200);
    });

    bool hasL = p->left!=nullptr, hasR=p->right!=nullptr;
    if (hasL && hasR) {
        ds::BTNode* a = p->left; while(a && a->right) a=a->right;
        steps.push_back([=](){
          std::function<void(ds::BTNode*, qreal, qreal, qreal)> g;
          g = [=,&g,drawPlain](ds::BTNode* n,qreal x,qreal y,qreal s){
            if(!n) return;
            if(n == p){
              if(n->left)  { qreal lx=x-s, ly=y+100; drawPlain(n->left,lx,ly,s/1.8); }
              if(n->right) { qreal rx=x+s, ry=y+100; drawPlain(n->right,rx,ry,s/1.8); }
              return;
            }
            bool hl = (n==a);
            view->addNode(x,y, QString::number(n->key), hl);
            if(n->left)  { qreal lx=x-s, ly=y+100; view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34)); g(n->left, lx,ly,s/1.8); }
            if(n->right) { qreal rx=x+s, ry=y+100; view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34)); g(n->right,rx,ry,s/1.8); }
          };
          view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：删除第3步 标红左子树中序最后一个结点 a")); g(bst.root(),400,120,200);
        });

        steps.push_back([=](){ bst.eraseKey(value); view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：删除第4步 接上并完成")); drawBT(bst.root(),400,120,200,0); });
        timer.start(); return;
    }

    steps.push_back([=](){ bst.eraseKey(value); view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：删除完成")); drawBT(bst.root(),400,120,200,0); });
    timer.start();
}

void MainWindow::bstClear() { bst.clear(); view->resetScene(); view->setTitle(QStringLiteral("BST（空）")); }

// ===== 哈夫曼树（含合并动画） =====
void MainWindow::huffmanBuild() {
    auto w = parseIntList(huffmanInput->text());
    if (w.isEmpty()) { view->resetScene(); view->setTitle(QStringLiteral("哈夫曼树：请输入权值序列")); showMessage(QStringLiteral("哈夫曼树：无有效输入")); return; }
    huff.clear(); timer.stop(); steps.clear(); stepIndex = 0;

    auto addEdgeLabel = [this](const QPointF& a, const QPointF& b, const QString& text, qreal offset = 12.0) {
        QPointF mid((a.x()+b.x())/2.0, (a.y()+b.y())/2.0);
        qreal vx = b.x() - a.x(), vy = b.y() - a.y(); qreal L  = std::sqrt(vx*vx + vy*vy);
        qreal nx = 0.0, ny = -1.0; if (L > 1e-6) { nx = -vy / L; ny =  vx / L; }
        QPointF pos = mid + QPointF(nx*offset, ny*offset);
        auto* t = view->Scene()->addText(text); t->setDefaultTextColor(QColor("#111")); QRectF tb = t->boundingRect(); t->setPos(pos.x() - tb.width()/2.0, pos.y() - tb.height()/2.0);
    };

    const qreal R = 34;
    using DrawFn = std::function<void(ds::BTNode*, qreal, qreal, qreal, const QString&, bool)>;
    auto drawHuffTree = std::make_shared<DrawFn>();
    *drawHuffTree = [this, drawHuffTree, addEdgeLabel, R](ds::BTNode* n, qreal x, qreal y, qreal dist, const QString& prefix, bool annotateCodes) {
        if (!n) return; bool isLeaf = (!n->left && !n->right);
        view->addNode(x, y, QString::number(n->key), isLeaf);
        if (n->left)  { qreal lx = x - dist, ly = y + 100; QPointF a(x,  y + R), b(lx, ly - R); view->addEdge(a, b); addEdgeLabel(a, b, "0", 12.0); (*drawHuffTree)(n->left,  lx, ly, dist/1.8, prefix + "0", annotateCodes); }
        if (n->right) { qreal rx = x + dist, ry = y + 100; QPointF a(x,  y + R), b(rx, ry - R); view->addEdge(a, b); addEdgeLabel(a, b, "1", 12.0); (*drawHuffTree)(n->right, rx, ry, dist/1.8, prefix + "1", annotateCodes); }
        if (annotateCodes && isLeaf) { QString code = prefix.isEmpty() ? QString("0") : prefix; auto* t = view->Scene()->addText(code); t->setDefaultTextColor(QColor("#065f46")); QRectF tb = t->boundingRect(); t->setPos(x - tb.width()/2.0, y - R - 12 - tb.height()); }
    };

    QVector<ds::BTNode*> forest; forest.reserve(w.size()); for (int x : w) forest.push_back(ds::Huffman::makeNode(x));

    auto drawForestFixed = [=](const QVector<ds::BTNode*>& F, const QString& title) {
        view->resetScene(); view->setTitle(title);
        qreal x = 150; for (int i = 0; i < F.size(); ++i) { (*drawHuffTree)(F[i], x, 120, 60, "", false); x += 180; }
    };

    auto tweenTwo = [=](const QVector<ds::BTNode*>& F, int i1, int i2, qreal t, const QString& title){
        view->resetScene(); view->setTitle(title);
        QVector<qreal> xs(F.size()); qreal x0 = 150; for (int i=0;i<F.size();++i){ xs[i] = x0 + i*180; }
        qreal mid = (xs[i1] + xs[i2]) / 2.0; qreal xi1 = lerp(xs[i1], mid-40, t); qreal xi2 = lerp(xs[i2], mid+40, t);
        for (int i=0;i<F.size();++i){ qreal x = xs[i]; if (i==i1) x = xi1; if (i==i2) x = xi2; (*drawHuffTree)(F[i], x, 120, 60, "", false); }
    };

    QVector<ds::BTNode*> cur = forest;
    steps.push_back([=]() { drawForestFixed(cur, QStringLiteral("哈夫曼树：初始森林（%1 棵）").arg(cur.size())); statusBar()->showMessage(QStringLiteral("哈夫曼树：开始构建")); });

    const int tweenFrames = 8; const int oldInterval = timer.interval();

    while (cur.size() > 1) {
        int i1 = -1, i2 = -1;
        for (int i = 0; i < cur.size(); ++i) if (i1 == -1 || cur[i]->key < cur[i1]->key) i1 = i;
        for (int i = 0; i < cur.size(); ++i) if (i != i1 && (i2 == -1 || cur[i]->key < cur[i2]->key)) i2 = i;
        if (i1 > i2) std::swap(i1, i2);

        int a = cur[i1]->key, b = cur[i2]->key;
        ds::BTNode* parent = ds::Huffman::makeNode(a + b);
        parent->left  = cur[i1]; parent->right = cur[i2];

        QVector<ds::BTNode*> before = cur;
        QVector<ds::BTNode*> after  = cur; after[i1] = parent; after.remove(i2);

        steps.push_back([=](){ drawForestFixed(before, QStringLiteral("哈夫曼树：选择最小两棵：%1 与 %2").arg(a).arg(b)); });

        for (int f=0; f<=tweenFrames; ++f){
            qreal t = qreal(f)/tweenFrames;
            steps.push_back([=](){ timer.setInterval(60); tweenTwo(before, i1, i2, t, QStringLiteral("哈夫曼树：合并中（移动）")); });
        }

        steps.push_back([=](){ drawForestFixed(after, QStringLiteral("哈夫曼树：合并 %1 + %2 -> %3").arg(a).arg(b).arg(parent->key)); timer.setInterval(oldInterval); });

        cur[i1] = parent; cur.remove(i2);
    }

    if (!cur.isEmpty()) huff.rootNode = cur[0];

    steps.push_back([=]() {
        view->resetScene(); view->setTitle(QStringLiteral("哈夫曼树：构建完成（边标 0/1；叶子上方显示码字）"));
        (*drawHuffTree)(huff.root(), 400, 120, 200, "", true);
        auto* legend = view->Scene()->addText(QStringLiteral("图例：黄色=原始叶结点   蓝绿色=内部结点（合并产生）"));
        legend->setDefaultTextColor(QColor("#444")); legend->setPos(16, 54);
        showMessage(QStringLiteral("哈夫曼树：完成"));
    });

    timer.start();
}

// ===== AVL树 =====
void MainWindow::avlBuild() {
    auto a = parseIntList(avlInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    steps.push_back([this]() { avl.clear(); view->resetScene(); view->setTitle(QStringLiteral("AVL树：开始构建")); drawBT(avl.root(), 400, 120, 200, 0); });
    for (int i = 0; i < a.size(); i++) {
        steps.push_back([=]() { avl.insert(a[i]); view->resetScene(); view->setTitle(QStringLiteral("AVL树：插入 %1（第 %2/%3 步）").arg(a[i]).arg(i+1).arg(a.size())); drawBT(avl.root(), 400, 120, 200, 0); });
    }
    timer.start();
}

void MainWindow::avlInsert() {
    bool ok = false; int value = avlValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("AVL树：请输入有效的键值")); return; }

    timer.stop(); steps.clear(); stepIndex = 0;

    // 显示插入前的树
    steps.push_back([=]() {
        view->resetScene();
        view->setTitle(QStringLiteral("AVL树：插入 %1 前").arg(value));
        drawBT(avl.root(), 400, 120, 200, 0);
    });

    // 插入操作
    steps.push_back([=]() {
        avl.insert(value);
        view->resetScene();
        view->setTitle(QStringLiteral("AVL树：插入 %1 后").arg(value));
        drawBT(avl.root(), 400, 120, 200, value);
        showMessage(QStringLiteral("AVL树：插入 %1 完成").arg(value));
    });

    timer.start();
}

void MainWindow::avlClear() {
    avl.clear();
    view->resetScene();
    view->setTitle(QStringLiteral("AVL树（空）"));
    showMessage(QStringLiteral("AVL树：已清空"));
}

void MainWindow::huffmanClear() { huff.clear(); view->resetScene(); view->setTitle(QStringLiteral("哈夫曼树（空）")); statusBar()->showMessage(QStringLiteral("哈夫曼树：已清空")); }

// ===== 绘制基础 =====
void MainWindow::drawSeqlist(const ds::Seqlist& sl){
    view->resetScene(); view->setTitle(QStringLiteral("顺序表"));
    const int n = sl.size(); const qreal cellW = 68, cellH = 54, gap = 14; const qreal startX = 80, startY = 180;
    for (int i=0;i<n;++i){
        qreal x = startX + i*(cellW+gap), y = startY;
        view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"), 2), QBrush(QColor("#e8eef9")));
        auto* label = view->Scene()->addText(QString::number(sl.get(i))); QRectF tb = label->boundingRect();
        label->setDefaultTextColor(Qt::black); label->setPos(x + (cellW - tb.width())/2, y + (cellH - tb.height())/2 - 1);
        auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray); idx->setPos(x + cellW/2 - 6, y + cellH + 6);
    }
}

void MainWindow::drawLinklist(const ds::Linklist& ll){
    view->resetScene(); view->setTitle(QStringLiteral("单链表"));
    auto* p = ll.gethead();
    qreal x=150, y=220, lastx=-1;
    int i = 0;

    // 绘制头指针 - 向左移动
    auto* headLabel = view->Scene()->addText("head");
    headLabel->setDefaultTextColor(QColor("#334155"));
    headLabel->setFont(QFont("Arial", 10, QFont::Bold));
    headLabel->setPos(30, y-10);

    if (p) {
        view->addEdge(QPointF(60, y), QPointF(x-35, y));
    }

    while(p) {
        // 使用改进的节点样式
        view->addNode(x, y, QString::number(ll.get(i)), false);

        // 改进的索引显示
        auto* idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(QColor("#64748b"));
        idx->setFont(QFont("Arial", 9));
        idx->setPos(x-8, y+45);

        // 绘制连接线
        if(lastx > 0) {
            view->addEdge(QPointF(lastx, y), QPointF(x-35, y));
        }

        lastx = x+35;
        x += 120;
        p = p->next;
        i++;
    }

    // 绘制尾指针 - 向右移动，避免被节点遮挡
    if (i > 0) {
        auto* tailLabel = view->Scene()->addText("tail");
        tailLabel->setDefaultTextColor(QColor("#334155"));
        tailLabel->setFont(QFont("Arial", 10, QFont::Bold));
        tailLabel->setPos(x-20, y-10); // 从x-30改为x-10，向右移动

        // 调整尾指针连接线，确保不被节点遮挡
        if (lastx > 0) {
            view->addEdge(QPointF(lastx, y), QPointF(x-30, y)); // 从x-85改为x-65
        }
    }
}

void MainWindow::drawStack(const ds::Stack& st){
    view->resetScene(); view->setTitle(QStringLiteral("顺序栈（U 型槽：自适应高度）"));
    QGraphicsScene* S = view->Scene();
    const qreal x0 = 380, y0 = 120, W = 300, T = 12, innerPad = 6;
    const qreal BLOCK_H = 32, GAP = 4;
    const int   n = st.size();
    const int   MIN_LEVELS = 6, levels = std::max(n, MIN_LEVELS);
    const qreal innerH = levels * BLOCK_H + (levels - 1) * GAP;
    const qreal H = innerH + T + BLOCK_H;
    QBrush wall(QColor("#334155")); QPen   none(Qt::NoPen);
    S->addRect(QRectF(x0, y0, T, H), none, wall);
    S->addRect(QRectF(x0 + W - T, y0, T, H), none, wall);
    S->addRect(QRectF(x0, y0 + H - T, W, T), none, wall);
    { auto* base = S->addText(QStringLiteral("栈底  BASE")); base->setDefaultTextColor(QColor("#475569")); QRectF bb = base->boundingRect(); base->setPos(x0 + W/2 - bb.width()/2, y0 + H + 8); }
    const qreal innerW = W - 2*T - 2*innerPad; const qreal leftX  = x0 + T + innerPad; const qreal bottomInnerY = y0 + H - T;
    QBrush boxFill(QColor("#93c5fd")), topFill(QColor("#60a5fa")); QPen boxPen(QColor("#1e293b")); boxPen.setWidthF(1.2);
    for (int i = 0; i < n; ++i) {
        const bool  isTop = (i == n - 1);
        const qreal yTop  = bottomInnerY - (i + 1) * BLOCK_H - i * GAP;
        S->addRect(QRectF(leftX, yTop, innerW, BLOCK_H), boxPen, isTop ? topFill : boxFill);
        auto* label = S->addText(QString::number(st.get(i))); label->setDefaultTextColor(Qt::black);
        QRectF tb = label->boundingRect(); label->setPos(leftX + innerW/2 - tb.width()/2, yTop + BLOCK_H/2 - tb.height()/2);
    }
    const qreal yTopBlock = bottomInnerY - n * BLOCK_H - (n - 1) * GAP; const QPointF target(leftX + innerW/2, yTopBlock);
    auto* t = S->addText("TOP"); t->setDefaultTextColor(Qt::red); QRectF tb = t->boundingRect(); const QPointF tagPos(x0 + W + 16, yTopBlock - tb.height()/2); t->setPos(tagPos);
    QPointF a(tagPos.x() + tb.width()/2, tagPos.y() + tb.height()/2); view->addEdge(a, target);
    S->setSceneRect(S->itemsBoundingRect().adjusted(-40, -40, 160, 80));
}

void MainWindow::drawBT(ds::BTNode* root, qreal x, qreal y, qreal distance, int highlightKey){
    if(!root) return;
    bool hl = (root->key == highlightKey);
    view->addNode(x, y, QString::number(root->key), hl);
    if(root->left)  { qreal lx = x - distance, ly = y + 100; view->addEdge(QPointF(x,y+34), QPointF(lx,ly-34)); drawBT(root->left,  lx, ly, distance/1.8, highlightKey); }
    if(root->right) { qreal rx = x + distance, ry = y + 100; view->addEdge(QPointF(x,y+34), QPointF(rx,ry-34)); drawBT(root->right, rx, ry, distance/1.8, highlightKey); }
}

void MainWindow::animateBTOrder(const int* order, int n, const QString& title)
{
    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 选择需要绘制的树根：标题包含"BST/二叉搜索树"则优先用 BST，否则用普通二叉树
    ds::BTNode* rootBT  = bt.root();
    ds::BTNode* rootBST = bst.root();
    ds::BTNode* root = nullptr;
    if (title.contains("BST", Qt::CaseInsensitive) || title.contains(QStringLiteral("二叉搜索树"))) {
        root = rootBST ? rootBST : rootBT;
    } else {
        root = rootBT ? rootBT : rootBST;
    }

    // 空序列或无树时的兜底
    if (!order || n <= 0 || !root) {
        view->resetScene();
        view->setTitle(title + QStringLiteral("（空）"));
        if (root) drawBT(root, 400, 120, 200, 0);
        timer.start(); // 保持流程一致
        return;
    }

    // 逐步高亮访问序列
    for (int i = 0; i < n; ++i) {
        const int key = order[i];
        steps.push_back([this, i, n, key, title, root]() {
            view->resetScene();
            view->setTitle(QString("%1：访问 %2（%3/%4）").arg(title).arg(key).arg(i + 1).arg(n));
            drawBT(root, 400, 120, 200, key); // 仅高亮，不移动
        });
    }
    timer.start();
}

// ================== 文件保存/打开/导出 ==================
void MainWindow::saveDoc() {
    QFileDialog dialog(this, QStringLiteral("保存为"), "", "DS Visualizer (*.dsviz)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("dsviz");
    dialog.setStyleSheet(
        "QFileDialog { background: #f8fafc; }"
        "QLabel { color: #334155; font-weight: 600; }"
        "QPushButton { background: #3b82f6; color: white; border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background: #2563eb; }"
        "QLineEdit { border: 2px solid #e2e8f0; border-radius: 6px; padding: 6px; }"
        "QLineEdit:focus { border-color: #3b82f6; }"
    );

    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.selectedFiles().first();
        if(path.isEmpty()) return;

        // 新格式：一次性保存所有数据结构的状态，方便下次直接恢复
        QJsonObject root;
        root["kind"] = "all";

        // 顺序表
        {
            QJsonObject obj;
            QJsonArray arr;
            for(int i = 0; i < seq.size(); ++i) arr.push_back(seq.get(i));
            obj["values"] = arr;
            root["seqlist"] = obj;
        }
        // 单链表
        {
            QJsonObject obj;
            QJsonArray arr;
            for(int i = 0; i < link.size(); ++i) arr.push_back(link.get(i));
            obj["values"] = arr;
            root["linkedlist"] = obj;
        }
        // 栈
        {
            QJsonObject obj;
            QJsonArray arr;
            for(int i = 0; i < st.size(); ++i) arr.push_back(st.get(i));
            obj["values"] = arr;
            root["stack"] = obj;
        }
        // 普通二叉树（层序 + 哨兵）
        {
            QJsonObject obj;
            int sent = (btLastNullSentinel_ == INT_MIN ? -1 : btLastNullSentinel_);
            obj["null"] = sent;
            QJsonArray arr;
            auto v = dumpBTLevel(bt.root(), sent);
            for(int x: v) arr.push_back(x);
            obj["level"] = arr;
            root["binarytree"] = obj;
        }
        // BST（先序）
        {
            QJsonObject obj;
            QJsonArray arr;
            QVector<int> pre;
            dumpPreorder(bst.root(), pre);
            for(int x: pre) arr.push_back(x);
            obj["preorder"] = arr;
            root["bst"] = obj;
        }
        // AVL（先序）
        {
            QJsonObject obj;
            QJsonArray arr;
            QVector<int> pre;
            dumpPreorder(avl.root(), pre);
            for(int x: pre) arr.push_back(x);
            obj["preorder"] = arr;
            root["avl"] = obj;
        }
        // Huffman（权值列表）
        {
            QJsonObject obj;
            QJsonArray arr;
            if(!huffLastWeights_.isEmpty()) {
                for(int w: huffLastWeights_) arr.push_back(w);
            } else {
                QVector<int> leaves;
                collectLeafWeights(huff.root(), leaves);
                for(int w: leaves) arr.push_back(w);
            }
            obj["weights"] = arr;
            root["huffman"] = obj;
        }

        QFile f(path);
        if(f.open(QIODevice::WriteOnly)) {
            f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
            f.close();
            statusBar()->showMessage(QString("已保存全部数据结构：%1").arg(path));
        }
    }
}

void MainWindow::openDoc() {
    QFileDialog dialog(this, QStringLiteral("打开"), "", "DS Visualizer (*.dsviz)");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setStyleSheet(
        "QFileDialog { background: #f8fafc; }"
        "QLabel { color: #334155; font-weight: 600; }"
        "QPushButton { background: #3b82f6; color: white; border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background: #2563eb; }"
        "QLineEdit { border: 2px solid #e2e8f0; border-radius: 6px; padding: 6px; }"
        "QLineEdit:focus { border-color: #3b82f6; }"
    );

    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.selectedFiles().first();
        if(path.isEmpty()) return;

        QFile f(path);
        if(!f.open(QIODevice::ReadOnly)) {
            statusBar()->showMessage("打开失败");
            return;
        }

        auto doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        if(!doc.isObject()) {
            statusBar()->showMessage("文件格式错误");
            return;
        }

        auto o = doc.object();
        QString k = o.value("kind").toString();

        // 向后兼容：老文件只保存一种结构（原实现即是按 currentKind_ 导出一种）
        if (k != "all") {
            if(k == "seqlist") {
                seq.clear();
                for(auto v : o["values"].toArray()) seq.insert(seq.size(), v.toInt());
                currentKind_ = DocKind::SeqList;
                drawSeqlist(seq);
                view->setTitle("顺序表（已从文件恢复）");
            } else if(k == "linkedlist") {
                link.clear();
                for(auto v : o["values"].toArray()) link.insert(link.size(), v.toInt());
                currentKind_ = DocKind::LinkedList;
                drawLinklist(link);
                view->setTitle("单链表（已从文件恢复）");
            } else if(k == "stack") {
                st.clear();
                for(auto v : o["values"].toArray()) st.push(v.toInt());
                currentKind_ = DocKind::Stack;
                drawStack(st);
                view->setTitle("顺序栈（已从文件恢复）");
            } else if(k == "binarytree") {
                int sent = o["null"].toInt(-1);
                auto ja = o["level"].toArray();
                QVector<int> a; a.reserve(ja.size());
                for(auto v: ja) a.push_back(v.toInt());
                bt.clear();
                bt.buildTree(a.data(), a.size(), sent);
                btLastNullSentinel_ = sent;
                currentKind_ = DocKind::BinaryTree;
                view->resetScene();
                view->setTitle("二叉树（已从文件恢复）");
                drawBT(bt.root(), 400, 120, 200, 0);
            } else if(k == "bst") {
                bst.clear();
                for(auto v : o["preorder"].toArray()) bst.insert(v.toInt());
                currentKind_ = DocKind::BST;
                view->resetScene();
                view->setTitle("BST（已从文件恢复）");
                drawBT(bst.root(), 400, 120, 200, 0);
            } else if(k == "avl") {
                avl.clear();
                for(auto v : o["preorder"].toArray()) avl.insert(v.toInt());
                currentKind_ = DocKind::AVL;
                view->resetScene();
                view->setTitle("AVL（已从文件恢复）");
                drawBT(avl.root(), 400, 120, 200, 0);
            } else if(k == "huffman") {
                huff.clear();
                QVector<int> w; for(auto v : o["weights"].toArray()) w.push_back(v.toInt());
                huff.buildFromWeights(w.data(), w.size());
                huffLastWeights_ = w;
                currentKind_ = DocKind::Huffman;
                view->resetScene();
                view->setTitle("哈夫曼树（已从文件恢复）");
                drawBT(huff.root(), 400, 120, 200, 0);
            } else {
                statusBar()->showMessage("未知 kind，无法打开");
                return;
            }
            statusBar()->showMessage(QString("已打开：%1").arg(path));
            return;
        }

        // 新格式：一次性恢复所有数据结构，但不强制切换右侧模块；画布按当前模块刷新
        if (o.contains("seqlist")) {
            QJsonObject s = o["seqlist"].toObject();
            seq.clear();
            for (auto v : s["values"].toArray()) seq.insert(seq.size(), v.toInt());
        }
        if (o.contains("linkedlist")) {
            QJsonObject s = o["linkedlist"].toObject();
            link.clear();
            for (auto v : s["values"].toArray()) link.insert(link.size(), v.toInt());
        }
        if (o.contains("stack")) {
            QJsonObject s = o["stack"].toObject();
            st.clear();
            for (auto v : s["values"].toArray()) st.push(v.toInt());
        }
        if (o.contains("binarytree")) {
            QJsonObject s = o["binarytree"].toObject();
            int sent = s["null"].toInt(-1);
            QVector<int> a; for (auto v : s["level"].toArray()) a.push_back(v.toInt());
            bt.clear(); bt.buildTree(a.data(), a.size(), sent);
            btLastNullSentinel_ = sent;
        }
        if (o.contains("bst")) {
            QJsonObject s = o["bst"].toObject();
            bst.clear(); for (auto v : s["preorder"].toArray()) bst.insert(v.toInt());
        }
        if (o.contains("avl")) {
            QJsonObject s = o["avl"].toObject();
            avl.clear(); for (auto v : s["preorder"].toArray()) avl.insert(v.toInt());
        }
        if (o.contains("huffman")) {
            QJsonObject s = o["huffman"].toObject();
            huff.clear();
            QVector<int> w; for (auto v : s["weights"].toArray()) w.push_back(v.toInt());
            if (!w.isEmpty()) { huff.buildFromWeights(w.data(), w.size()); huffLastWeights_ = w; }
        }

        // 依据当前右侧模块选择刷新画布到该模块的“上一次状态”
        onModuleChanged(moduleCombo ? moduleCombo->currentIndex() : 0);
        statusBar()->showMessage(QString("已打开（全部数据结构已恢复）：%1").arg(path));
    }
}

// 新增：模块切换时同步画布为对应数据结构的上一次状态（若无则显示“空”）
void MainWindow::onModuleChanged(int index) {
    // 停止动画，直接展示该模块最近一次的状态
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene();

    switch (index) {
    case 0: // 顺序表
        currentKind_ = DocKind::SeqList;
        if (seq.size() > 0) { drawSeqlist(seq); view->setTitle(QStringLiteral("顺序表")); }
        else { view->setTitle(QStringLiteral("顺序表（空）")); }
        break;
    case 1: // 单链表
        currentKind_ = DocKind::LinkedList;
        if (link.size() > 0) { drawLinklist(link); view->setTitle(QStringLiteral("单链表")); }
        else { view->setTitle(QStringLiteral("单链表（空）")); }
        break;
    case 2: // 栈
        currentKind_ = DocKind::Stack;
        if (st.size() > 0) { drawStack(st); view->setTitle(QStringLiteral("顺序栈")); }
        else { view->setTitle(QStringLiteral("顺序栈（空）")); }
        break;
    case 3: // 普通二叉树
        currentKind_ = DocKind::BinaryTree;
        if (bt.root()) { drawBT(bt.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("二叉树")); }
        else { view->setTitle(QStringLiteral("二叉树（空）")); }
        break;
    case 4: // BST
        currentKind_ = DocKind::BST;
        if (bst.root()) { drawBT(bst.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("BST")); }
        else { view->setTitle(QStringLiteral("BST（空）")); }
        break;
    case 5: // Huffman
        currentKind_ = DocKind::Huffman;
        if (huff.root()) { drawBT(huff.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("哈夫曼树")); }
        else { view->setTitle(QStringLiteral("哈夫曼树（空）")); }
        break;
    case 6: // AVL
        currentKind_ = DocKind::AVL;
        if (avl.root()) { drawBT(avl.root(), 400, 120, 200, 0); view->setTitle(QStringLiteral("AVL")); }
        else { view->setTitle(QStringLiteral("AVL（空）")); }
        break;
    case 7: // DSL（不绑定具体结构）
        currentKind_ = DocKind::None;
        view->setTitle(QStringLiteral("脚本/DSL"));
        break;
    default:
        break;
    }
}

void MainWindow::exportPNG() {
    QFileDialog dialog(this, QStringLiteral("导出为 PNG"), "", "PNG Image (*.png)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("png");
    dialog.setStyleSheet(
        "QFileDialog { background: #f8fafc; }"
        "QLabel { color: #334155; font-weight: 600; }"
        "QPushButton { background: #3b82f6; color: white; border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background: #2563eb; }"
        "QLineEdit { border: 2px solid #e2e8f0; border-radius: 6px; padding: 6px; }"
        "QLineEdit:focus { border-color: #3b82f6; }"
    );

    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.selectedFiles().first();
        if(path.isEmpty()) return;

        auto* sc = view->Scene();
        QRectF rect = sc->itemsBoundingRect().adjusted(-40, -40, 40, 80);
        QImage img(rect.size().toSize() * 2, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.translate(-rect.topLeft() * 2);
        p.scale(2, 2);
        sc->render(&p, QRectF(), rect);
        p.end();

        if(img.save(path))
            statusBar()->showMessage(QString("已导出 PNG：%1").arg(path));
        else
            statusBar()->showMessage("导出 PNG 失败");
    }

}

// ================== DSL / 自然语言 ==================
void MainWindow::insertDSLExample() {
    // 弹出一个只读文档页面，详细说明 DSL 用法
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle(QStringLiteral("DSL 使用说明"));
    dlg->resize(780, 620);
    dlg->setModal(false);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);

    auto* v = new QVBoxLayout(dlg);

    auto* doc = new QTextBrowser(dlg);
    doc->setOpenExternalLinks(false);
    doc->setStyleSheet(
        "QTextBrowser{border:2px solid #e2e8f0;border-radius:10px;padding:12px;background:white;}"
    );

    const QString html = QString::fromUtf8(R"HTML(
<h2 style="margin:0 0 12px 0;">DSL 使用说明</h2>
<p>本 DSL 只做“解析 → 设置控件 → 调用已有槽函数”，<b>动画与绘制复用现有实现</b>。规则：</p>
<ul>
  <li>每行一条命令；<code>#</code> 开头为注释；大小写不敏感；数字可用空格或逗号分隔。</li>
  <li>普通二叉树层序构建支持哨兵：<code>null=-1</code>，可省略（默认 -1）。</li>
  <li>命令按顺序逐条执行；一条命令的动画结束后自动进入下一条。</li>
</ul>

<h3>顺序表（Seqlist）</h3>
<pre><code>seq 1 3 5 7
seq.insert pos value
seq.erase pos
seq.clear
</code></pre>

<h3>单链表（Linklist）</h3>
<pre><code>link 2 4 6 8
link.insert pos value
link.erase pos
link.clear
</code></pre>

<h3>顺序栈（Stack）</h3>
<pre><code>stack 3 8 13
stack.push value
stack.pop
stack.clear
</code></pre>

<h3>普通二叉树（Binary Tree，层序+哨兵）</h3>
<pre><code>bt 15 6 23 4 -1 -1 7  null=-1
bt.preorder
bt.inorder
bt.postorder
bt.levelorder
bt.clear
</code></pre>

<h3>二叉搜索树（BST）</h3>
<pre><code>bst 15 6 23 4 7 17 71
bst.find x
bst.insert x
bst.erase x
bst.clear
</code></pre>

<h3>哈夫曼树（Huffman）</h3>
<pre><code>huff 5 9 12 13 16 45
huff.clear
</code></pre>

<h3>AVL 树</h3>
<pre><code>avl 10 20 30 40 50 25
avl.insert x
avl.clear
</code></pre>

<hr/>
<p style="color:#6b7280">小贴士：若要批量运行多结构示例，可直接把右下角“插入示例”粘贴到编辑框后点击“执行脚本”。</p>
)HTML");

    doc->setHtml(html);

    // 一键写入示例脚本
    const QString sample = QString::fromUtf8(
R"(# —— 顺序表 ——
seq 1 3 5 7
seq.insert 2 99
seq.erase 1
seq.clear

# —— 单链表 ——
link 2 4 6 8
link.insert 1 42
link.erase 3
link.clear

# —— 栈 ——
stack 3 8 13
stack.push 21
stack.pop
stack.clear

# —— 普通二叉树（层序 + 哨兵）——
bt 15 6 23 4 -1 -1 7  null=-1
bt.preorder
bt.inorder
bt.postorder
bt.levelorder
bt.clear

# —— BST ——
bst 15 6 23 4 7 17 71
bst.find 7
bst.insert 50
bst.erase 23
bst.clear

# —— Huffman ——
huff 5 9 12 13 16 45
huff.clear

# —— AVL ——
avl 10 20 30 40 50 25
avl.insert 35
avl.clear
)");

    auto* btnBar = new QHBoxLayout;
    auto* btnClose = new QPushButton(QStringLiteral("关闭"));
    btnClose->setStyleSheet("QPushButton{background:#ef4444;color:white;}");

    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::close);

    btnBar->addStretch(1);
    btnBar->addWidget(btnClose);

    v->addWidget(doc, 1);
    v->addLayout(btnBar);

    dlg->show();
}

void MainWindow::runDSL() {
    // 读取并按行分割
    const QString all = dslEdit->toPlainText();
    QStringList lines = all.split('\n', Qt::SkipEmptyParts);

    // =============== 整体校验：一次只能操作一种数据结构 ===============
    {
        QSet<QString> families;
        for (const QString& rawLine : lines) {
            QString s = rawLine.trimmed().toLower();
            if (s.isEmpty()) continue;
            QRegularExpression headRe("^\\s*([a-z]+)");
            auto mm = headRe.match(s);
            if (!mm.hasMatch()) continue;
            const QString head = mm.captured(1);
            if (head == "seq" || head == "link" || head == "stack" ||
                head == "bt"  || head == "bst"  || head == "huff" || head == "avl") {
                families.insert(head);
            }
        }
        if (families.size() > 1) {
            QStringList lst; for (const auto& f : families) lst << f;
            QMessageBox::warning(this, QStringLiteral("输入有误"),
                                 QStringLiteral("检测到 DSL 同时包含多种数据结构的操作（%1）。当前版本每次仅支持一种数据结构，请修改后重新输入。")
                                     .arg(lst.join(", ")));
            statusBar()->showMessage(QStringLiteral("DSL 校验失败：包含多种数据结构"));
            return;
        }
    }

    // =============== 单行多指令校验（不允许一行包含两条或以上指令） ===============
    static const QRegularExpression kCmdTokenRe(
        R"((?<![a-z])(seq|link|stack|bt|bst|huff|avl)(?:\.[a-z]+)?(?![a-z]))",
        QRegularExpression::CaseInsensitiveOption
    );
    for (const QString& rawLine : lines) {
        const QString s = rawLine.trimmed();
        if (s.isEmpty()) continue;

        const bool hasBadSep = s.contains(';') || s.contains(u'；')
                            || s.contains("&&") || s.contains("||")
                            || s.contains(u'、') || s.contains(u'，');

        int cmdCount = 0;
        for (auto it = kCmdTokenRe.globalMatch(s); it.hasNext(); it.next()) ++cmdCount;

        if (hasBadSep || cmdCount >= 2) {
            QMessageBox::warning(this, QStringLiteral("输入有误"),
                                 QStringLiteral("DSL 每行仅能包含一条指令，请拆分后重新输入。\n问题行：%1")
                                     .arg(rawLine.trimmed()));
            statusBar()->showMessage(QStringLiteral("DSL 校验失败：单行包含多条指令"));
            return;
        }
    }

    // =============== 解析与执行计划 ===============
    QVector<std::function<void()>> ops;
    ops.reserve(lines.size());

    auto asNumbers = [this](const QString& s)->QVector<int>{
        return parseIntList(s); // 已有：正则抽取所有整数，支持空格/逗号
    };

    auto normalized = [](QString s){
        s = s.trimmed();
        s.replace(QRegularExpression("[\\t,]+"), " "); // 制表符/逗号 → 空格
        return s.toLower();
    };

    for (const QString& ln : lines) {
        QString s = normalized(ln);
        if (s.isEmpty()) continue;

        // ================= 顺序表 =================
        if (s.startsWith("seq.insert")) {
            // seq.insert pos val
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 3) {
                bool ok1=false, ok2=false;
                int pos = tokens.value(1).toInt(&ok1);
                int val = tokens.value(2).toInt(&ok2);
                if (ok1 && ok2) {
                    ops.push_back([=](){
                        currentKind_ = DocKind::SeqList;
                        seqlistPosition->setValue(pos);
                        seqlistValue->setText(QString::number(val));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        seqlistInsert();
                    });
                    continue;
                }
            }
        }
        if (s.startsWith("seq.erase")) {
            // seq.erase pos
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int pos = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=](){
                        currentKind_ = DocKind::SeqList;
                        seqlistPosition->setValue(pos);
                        timer.stop(); steps.clear(); stepIndex = 0;
                        seqlistErase();
                    });
                    continue;
                }
            }
        }
        if (s == "seq.clear") {
            ops.push_back([=](){
                currentKind_ = DocKind::SeqList;
                timer.stop(); steps.clear(); stepIndex = 0;
                seqlistClear();
            });
            continue;
        }
        if (s.startsWith("seq ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=](){
                currentKind_ = DocKind::SeqList;
                seqlistInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                seqlistBuild();
            });
            continue;
        }

        // ================= 单链表 =================
        if (s.startsWith("link.insert")) {
            // link.insert pos val
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 3) {
                bool ok1=false, ok2=false;
                int pos = tokens.value(1).toInt(&ok1);
                int val = tokens.value(2).toInt(&ok2);
                if (ok1 && ok2) {
                    ops.push_back([=](){
                        currentKind_ = DocKind::LinkedList;
                        linklistPosition->setValue(pos);
                        linklistValue->setText(QString::number(val));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        linklistInsert();
                    });
                    continue;
                }
            }
        }
        if (s.startsWith("link.erase")) {
            // link.erase pos
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int pos = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=](){
                        currentKind_ = DocKind::LinkedList;
                        linklistPosition->setValue(pos);
                        timer.stop(); steps.clear(); stepIndex = 0;
                        linklistErase();
                    });
                    continue;
                }
            }
        }
        if (s == "link.clear") {
            ops.push_back([=](){
                currentKind_ = DocKind::LinkedList;
                timer.stop(); steps.clear(); stepIndex = 0;
                linklistClear();
            });
            continue;
        }
        if (s.startsWith("link ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=](){
                currentKind_ = DocKind::LinkedList;
                linklistInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                linklistBuild();
            });
            continue;
        }

        // ================= 栈 =================
        if (s.startsWith("stack.push")) {
            // stack.push v
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int v = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=](){
                        currentKind_ = DocKind::Stack;
                        stackValue->setText(QString::number(v));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        stackPush();
                    });
                    continue;
                }
            }
        }
        if (s == "stack.pop") {
            ops.push_back([=](){
                currentKind_ = DocKind::Stack;
                timer.stop(); steps.clear(); stepIndex = 0;
                stackPop();
            });
            continue;
        }
        if (s == "stack.clear") {
            ops.push_back([=](){
                currentKind_ = DocKind::Stack;
                timer.stop(); steps.clear(); stepIndex = 0;
                stackClear();
            });
            continue;
        }
        if (s.startsWith("stack ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=](){
                currentKind_ = DocKind::Stack;
                stackInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                stackBuild();
            });
            continue;
        }

        // ================= 二叉树（bt）=================
        if (s == "bt.clear") {
            ops.push_back([=](){
                currentKind_ = DocKind::BinaryTree;
                timer.stop(); steps.clear(); stepIndex = 0;
                btClear();
            });
            continue;
        }
        if (s.startsWith("bt.preorder"))   { ops.push_back([=](){ timer.stop(); steps.clear(); stepIndex=0; btPreorder();   }); continue; }
        if (s.startsWith("bt.inorder"))    { ops.push_back([=](){ timer.stop(); steps.clear(); stepIndex=0; btInorder();    }); continue; }
        if (s.startsWith("bt.postorder"))  { ops.push_back([=](){ timer.stop(); steps.clear(); stepIndex=0; btPostorder();  }); continue; }
        if (s.startsWith("bt.levelorder")) { ops.push_back([=](){ timer.stop(); steps.clear(); stepIndex=0; btLevelorder(); }); continue; }

        if (s.startsWith("bt ")) {
            // 支持 bt ... null=x（默认 -1）
            QRegularExpression mNull(R"(null\s*=\s*(-?\d+))");
            auto m = mNull.match(s);
            int sentinel = -1;
            if (m.hasMatch()) sentinel = m.captured(1).toInt();

            auto a = asNumbers(s);
            // 若末尾是哨兵值且刚好是通过 null= 指定的，把它从序列里去掉
            if (m.hasMatch() && !a.isEmpty() && a.back() == sentinel) a.pop_back();

            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            const int nullSent = sentinel;

            ops.push_back([=](){
                currentKind_ = DocKind::BinaryTree;
                btInput->setText(numbers);
                btNull->setValue(nullSent);
                timer.stop(); steps.clear(); stepIndex = 0;
                btBuild();
            });
            continue;
        }

        // ================= BST =================
        if (s == "bst.clear") {
            ops.push_back([=](){
                currentKind_ = DocKind::BST;
                timer.stop(); steps.clear(); stepIndex = 0;
                bstClear();
            });
            continue;
        }
        if (s.startsWith("bst.find") || s.startsWith("bst.insert") || s.startsWith("bst.erase")) {
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int v = tokens.value(1).toInt(&ok);
                if (ok) {
                    if (tokens[0] == "bst.find") {
                        ops.push_back([=](){
                            currentKind_ = DocKind::BST;
                            bstValue->setText(QString::number(v));
                            timer.stop(); steps.clear(); stepIndex = 0;
                            bstFind();
                        });
                        continue;
                    }
                    if (tokens[0] == "bst.insert") {
                        ops.push_back([=](){
                            currentKind_ = DocKind::BST;
                            bstValue->setText(QString::number(v));
                            timer.stop(); steps.clear(); stepIndex = 0;
                            bstInsert();
                        });
                        continue;
                    }
                    if (tokens[0] == "bst.erase") {
                        ops.push_back([=](){
                            currentKind_ = DocKind::BST;
                            bstValue->setText(QString::number(v));
                            timer.stop(); steps.clear(); stepIndex = 0;
                            bstErase();
                        });
                        continue;
                    }
                }
            }
        }
        if (s.startsWith("bst ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=](){
                currentKind_ = DocKind::BST;
                bstInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                bstBuild();
            });
            continue;
        }

        // ================= Huffman =================
        if (s == "huff.clear") {
            ops.push_back([=](){
                currentKind_ = DocKind::Huffman;
                timer.stop(); steps.clear(); stepIndex = 0;
                huffmanClear();
            });
            continue;
        }
        if (s.startsWith("huff ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=](){
                currentKind_ = DocKind::Huffman;
                huffmanInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                huffmanBuild();
            });
            continue;
        }

        // ================= AVL =================
        if (s == "avl.clear") {
            ops.push_back([=](){
                currentKind_ = DocKind::AVL;
                timer.stop(); steps.clear(); stepIndex = 0;
                avlClear();
            });
            continue;
        }
        if (s.startsWith("avl.insert")) {
            auto tokens = s.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 2) {
                bool ok=false; int v = tokens.value(1).toInt(&ok);
                if (ok) {
                    ops.push_back([=](){
                        currentKind_ = DocKind::AVL;
                        avlValue->setText(QString::number(v));
                        timer.stop(); steps.clear(); stepIndex = 0;
                        avlInsert();
                    });
                    continue;
                }
            }
        }
        if (s.startsWith("avl ")) {
            auto a = asNumbers(s);
            QString numbers; for (int i=0;i<a.size();++i){ if(i) numbers+=' '; numbers+=QString::number(a[i]); }
            ops.push_back([=](){
                currentKind_ = DocKind::AVL;
                avlInput->setText(numbers);
                timer.stop(); steps.clear(); stepIndex = 0;
                avlBuild();
            });
            continue;
        }

        // 未识别：给出提示，不中断其它行
        ops.push_back([=](){ showMessage(QStringLiteral("未识别 DSL：%1").arg(ln.trimmed())); });
    }

    if (ops.isEmpty()) {
        showMessage(QStringLiteral("DSL：没有可执行的命令"));
        return;
    }

    // =============== 串行执行：每条命令的动画最后接“继续下一条” ===============
    auto runNext = std::make_shared<std::function<void()>>();
    *runNext = [this, ops, runNext]() mutable {
        if (ops.isEmpty()) {
            showMessage(QStringLiteral("DSL：执行完成"));
            return;
        }
        auto op = ops.takeFirst();
        op();
        QTimer::singleShot(0, this, [this, runNext](){
            if (steps.isEmpty()) {
                (*runNext)();
            } else {
                steps.push_back([runNext](){ (*runNext)(); });
                if (!timer.isActive()) timer.start();
            }
        });
    };

    // 开始执行第一条
    (*runNext)();
}



void MainWindow::runNLI() {
    const QString raw = nliEdit->toPlainText();
    const QString low = raw.trimmed().toLower();
    if (low.isEmpty()) { showMessage(QStringLiteral("NLI：请输入自然语言指令")); return; }

    // —— 仅检测 NLI 内部是否混用了多种数据结构 ——
    QSet<QString> hits;
    auto hitIf = [&](const QString& key, std::initializer_list<QString> kws){
        for (const auto& k : kws) if (low.contains(k)) { hits.insert(key); break; }
    };
    hitIf("seq",  {"顺序表","顺序","seqlist","seq"});
    hitIf("link", {"链表","linklist","link"});
    hitIf("stack",{"栈","stack"});
    hitIf("bt",   {"二叉树","普通二叉树","binary tree","bt"});
    hitIf("bst",  {"二叉搜索树","binary search tree","bst"});
    hitIf("huff", {"哈夫曼","huffman","huff"});
    hitIf("avl",  {"avl"});

    if (hits.size() > 1) {
        // 拼接提示用的家族名（Qt6：QSet没有toList，这里手动拼）
        QStringList fam; for (const auto& s : hits) fam << s;
        QMessageBox::warning(this, QStringLiteral("输入不合法"),
            QStringLiteral("NLI：同一条指令内只能包含一种数据结构（检测到：%1），请重新输入。").arg(fam.join(", ")));
        return;
    }
    if (hits.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("未识别"),
            QStringLiteral("NLI：未能识别数据结构类型，请补充如“顺序表/链表/栈/二叉树/BST/哈夫曼/AVL”等关键词。"));
        return;
    }
    const QString kind = *hits.begin();

    // 提取所有整数（支持空格/逗号/混排）
    const QVector<int> nums = parseIntList(low);
    auto joinNums = [&](const QVector<int>& a){
        QString s; for (int i=0;i<a.size();++i){ if(i) s+=' '; s+=QString::number(a[i]); } return s;
    };
    auto hasAny = [&](std::initializer_list<QString> kws)->bool{
        for (const auto& k : kws) if (low.contains(k)) return true;
        return false;
    };

    QString dsl;  // 输出的 DSL 一行

    if (kind == "seq") {
        if (hasAny({"清空","清除","clear"})) { dsl = "seq.clear"; }
        else if (hasAny({"插入","插","insert"})) {
            if (nums.size() >= 2) dsl = QString("seq.insert %1 %2").arg(nums[0]).arg(nums[1]);
        } else if (hasAny({"删除","删","移除","erase","remove"})) {
            if (nums.size() >= 1) dsl = QString("seq.erase %1").arg(nums[0]);
        } else if (!nums.isEmpty()) {
            dsl = "seq " + joinNums(nums);
        }
    }
    else if (kind == "link") {
        if (hasAny({"清空","清除","clear"})) { dsl = "link.clear"; }
        else if (hasAny({"插入","插","insert"})) {
            if (nums.size() >= 2) dsl = QString("link.insert %1 %2").arg(nums[0]).arg(nums[1]);
        } else if (hasAny({"删除","删","移除","erase","remove"})) {
            if (nums.size() >= 1) dsl = QString("link.erase %1").arg(nums[0]);
        } else if (!nums.isEmpty()) {
            dsl = "link " + joinNums(nums);
        }
    }
    else if (kind == "stack") {
        if (hasAny({"清空","清除","clear"})) { dsl = "stack.clear"; }
        else if (hasAny({"出栈","弹栈","pop"})) { dsl = "stack.pop"; }
        else if (hasAny({"入栈","压栈","push","加入","添加"})) {
            if (nums.size() >= 1) dsl = QString("stack.push %1").arg(nums[0]);
        } else if (!nums.isEmpty()) {
            dsl = "stack " + joinNums(nums);
        }
    }
    else if (kind == "bt") {
        if (hasAny({"先序","前序","preorder"}))      dsl = "bt.preorder";
        else if (hasAny({"中序","inorder"}))          dsl = "bt.inorder";
        else if (hasAny({"后序","postorder"}))        dsl = "bt.postorder";
        else if (hasAny({"层序","层次","广度","levelorder"})) dsl = "bt.levelorder";
        else if (hasAny({"清空","清除","clear"}))     dsl = "bt.clear";
        else if (!nums.isEmpty())                     dsl = "bt " + joinNums(nums) + " null=-1";
    }
    else if (kind == "bst") {
        if (hasAny({"清空","清除","clear"})) { dsl = "bst.clear"; }
        else if (hasAny({"查找","寻找","搜索","find","search"})) {
            if (nums.size() >= 1) dsl = QString("bst.find %1").arg(nums[0]);
        } else if (hasAny({"插入","插","加入","添加","insert","add"})) {
            if (nums.size() >= 1) dsl = QString("bst.insert %1").arg(nums[0]);
        } else if (hasAny({"删除","删","移除","erase","remove"})) {
            if (nums.size() >= 1) dsl = QString("bst.erase %1").arg(nums[0]);
        } else if (!nums.isEmpty()) {
            dsl = "bst " + joinNums(nums);
        }
    }
    else if (kind == "huff") {
        if (hasAny({"清空","清除","clear"})) { dsl = "huff.clear"; }
        else if (!nums.isEmpty())           { dsl = "huff " + joinNums(nums); }
    }
    else if (kind == "avl") {
        if (hasAny({"清空","清除","clear"})) { dsl = "avl.clear"; }
        else if (hasAny({"插入","插","加入","添加","insert","add"})) {
            if (nums.size() >= 1) dsl = QString("avl.insert %1").arg(nums[0]);
        } else if (!nums.isEmpty()) {
            dsl = "avl " + joinNums(nums);
        }
    }

    if (dsl.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("信息不足"),
            QStringLiteral("NLI：无法从该句生成 DSL，请补充必要的信息（例如位置/值/遍历方式等）。"));
        return;
    }

    // 写回 DSL 并直接执行
    dslEdit->setPlainText(dsl);
    statusBar()->showMessage(QStringLiteral("NLI → DSL：%1").arg(dsl));
    runDSL();  // 依赖 DSL 端已有的完整执行逻辑（插入/删除/查找/遍历/清空等）
}

// ================== 辅助函数 ==================
QVector<int> MainWindow::dumpBTLevel(ds::BTNode* root, int nullSentinel) const {
    QVector<int> level;
    if(!root) return level;

    QVector<ds::BTNode*> q;
    q.reserve(128);
    q.push_back(root);
    int qi = 0, lastNonNull = -1;

    while(qi < q.size()) {
        ds::BTNode* p = q[qi++];
        if(p) {
            level.push_back(p->key);
            lastNonNull = level.size() - 1;
            q.push_back(p->left);
            q.push_back(p->right);
        } else {
            level.push_back(nullSentinel);
            q.push_back(nullptr);
            q.push_back(nullptr);
        }
        if(q.size() > 4096) break;
    }

    while(level.size() > 0 && level.back() == nullSentinel)
        level.pop_back();
    return level;
}

void MainWindow::dumpPreorder(ds::BTNode* r, QVector<int>& out) const {
    if(!r) return;
    out.push_back(r->key);
    dumpPreorder(r->left, out);
    dumpPreorder(r->right, out);
}

void MainWindow::collectLeafWeights(ds::BTNode* r, QVector<int>& out) const {
    if(!r) return;
    if(!r->left && !r->right) out.push_back(r->key);
    collectLeafWeights(r->left, out);
    collectLeafWeights(r->right, out);
}