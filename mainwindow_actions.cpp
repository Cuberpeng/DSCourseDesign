#include "mainwindow.h"
#include <QGraphicsScene>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QStatusBar>
#include <cmath>
#include <memory>
#include <climits>

static inline qreal lerp(qreal a, qreal b, qreal t){ return a + (b - a) * t; }

// ===== 顺序表 =====
void MainWindow::seqlistBuild(){
    seq.clear();
    auto a = parseIntList(seqlistInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene();
    view->setTitle(QStringLiteral("顺序表：建立"));
    steps.push_back([this](){ drawSeqlist(seq); statusBar()->showMessage(QStringLiteral("顺序表：开始建立")); });
    for (int x : a){
        steps.push_back([this, x](){ seq.insert(seq.size(), x); drawSeqlist(seq); statusBar()->showMessage(QStringLiteral("顺序表：插入 %1").arg(x)); });
    }
    timer.start();
}

void MainWindow::seqlistInsert(){
    int pos = seqlistPosition->value();
    bool ok = false; int val = seqlistValue->text().toInt(&ok);
    if(!ok) { statusBar()->showMessage(QStringLiteral("顺序表：请输入有效的值")); return; }

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

    for (int f = 0; f <= frames; ++f) {
        const double t  = double(f) / frames;
        const double dx = t * (cellW + gap);
        const int m = n + 1;
        steps.push_back([=](){
            if (f == 0) timer.setInterval(animInterval);
            view->resetScene(); view->setTitle(QStringLiteral("顺序表：插入 Step 1（pos=%1）").arg(pos));
            for (int i=0;i<m;++i){
                const bool isSource = (i >= pos && i <= m-2);
                if (!isSource){
                    qreal x = startX + i*(cellW+gap), y = startY;
                    view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#e8eef9")));
                    if (i < pos && i < arr.size() && !arr[i].isEmpty()){
                        auto* tItem = view->Scene()->addText(arr[i]); tItem->setDefaultTextColor(Qt::black);
                        auto r = tItem->boundingRect(); tItem->setPos(x + (cellW-r.width())/2, y + (cellH-r.height())/2 - 1);
                    }
                }
                qreal x = startX + i*(cellW+gap), y = startY;
                auto* idx = view->Scene()->addText(QString::number(i));
                idx->setDefaultTextColor(Qt::darkGray);
                idx->setPos(x+cellW/2-6, y+cellH+6);
            }
            for (int i=pos;i<m-1;++i){
                qreal sx = startX + i*(cellW+gap) + dx;
                qreal sy = startY;
                view->Scene()->addRect(QRectF(sx,sy,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#ffd166")));
                auto* moving = view->Scene()->addText(arr[i]); moving->setDefaultTextColor(Qt::black);
                auto r = moving->boundingRect(); moving->setPos(sx+(cellW-r.width())/2, sy+(cellH-r.height())/2-1);
            }
            statusBar()->showMessage(QStringLiteral("顺序表：插入 Step 1 - 高亮并整体右移"));
        });
    }

    steps.push_back([=](){
        seq.insert(pos, val);
        view->resetScene(); view->setTitle(QStringLiteral("顺序表：插入 Step 2（pos=%1, val=%2）").arg(pos).arg(val));
        const int m = n + 1;
        for (int i=0;i<m;++i){
            bool hi = (i==pos);
            qreal x = startX + i*(cellW+gap), y = startY;
            view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(hi? QColor("#ffd166"): QColor("#e8eef9")));
            QString s;
            if (i < pos && i < n) s = arr[i];
            else if (i == pos) s = QString::number(val);
            else if (i > pos && (i-1) < n) s = arr[i-1];
            auto* tItem = view->Scene()->addText(s); tItem->setDefaultTextColor(Qt::black);
            auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
            auto* idx = view->Scene()->addText(QString::number(i));
            idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x+cellW/2-6, y+cellH+6);
        }
        statusBar()->showMessage(QStringLiteral("顺序表：插入 Step 2 - 写入新值"));
        timer.setInterval(prevInterval);
    });

    timer.start();
}

void MainWindow::seqlistErase(){
    int pos = seqlistPosition->value();
    const int n = seq.size();
    if (pos < 0 || pos >= n) { statusBar()->showMessage(QStringLiteral("顺序表：删除失败(位置越界)")); return; }

    QVector<QString> arr(n);
    for (int i=0;i<n;++i) arr[i] = QString::number(seq.get(i));

    const qreal cellW = 68, cellH = 54, gap = 14;
    const qreal startX = 80, startY = 180;
    const int prevInterval = timer.interval();
    int frames = 10; if (n - 1 - pos > 300) frames = 6; if (n - 1 - pos > 800) frames = 4;
    const int animInterval = 60;

    timer.stop(); steps.clear(); stepIndex=0;

    steps.push_back([=](){
        timer.setInterval(animInterval);
        view->resetScene(); view->setTitle(QStringLiteral("顺序表：删除 Step 1（pos=%1）").arg(pos));
        for (int i=0;i<n;++i){
            bool hi = (i==pos);
            qreal x = startX + i*(cellW+gap), y = startY;
            view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(hi? QColor("#ffd166"): QColor("#e8eef9")));
            QString s = (i==pos ? "" : arr[i]);
            auto* t = view->Scene()->addText(s); t->setDefaultTextColor(Qt::black);
            auto r=t->boundingRect(); t->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
            auto* idx = view->Scene()->addText(QString::number(i));
            idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x+cellW/2-6, y+cellH+6);
        }
        statusBar()->showMessage(QStringLiteral("顺序表：删除 Step 1 - 高亮空位"));
    });

    for (int f = 0; f <= frames; ++f) {
        const double t  = double(f) / frames;
        const double dx = -t * (cellW + gap);
        const bool last = (f==frames);
        steps.push_back([=](){
            view->resetScene(); view->setTitle(QStringLiteral("顺序表：删除 Step 2（pos=%1）").arg(pos));
            if (!last){
                for (int i=0;i<n;++i){
                    bool isSource = (i >= pos+1);
                    if (!isSource){
                        qreal x = startX + i*(cellW+gap), y = startY;
                        view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#e8eef9")));
                        if (i < pos){
                            auto* tItem = view->Scene()->addText(arr[i]); tItem->setDefaultTextColor(Qt::black);
                            auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
                        }
                    }
                    qreal x = startX + i*(cellW+gap), y = startY;
                    auto* idx = view->Scene()->addText(QString::number(i));
                    idx->setDefaultTextColor(Qt::darkGray);
                    idx->setPos(x+cellW/2-6, y+cellH+6);
                }
                for (int i=pos+1;i<n;++i){
                    qreal sx = startX + i*(cellW+gap) + dx;
                    qreal sy = startY;
                    view->Scene()->addRect(QRectF(sx,sy,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(QColor("#ffd166")));
                    auto* moving = view->Scene()->addText(arr[i]);
                    moving->setDefaultTextColor(Qt::black);
                    auto r=moving->boundingRect();
                    moving->setPos(sx+(cellW-r.width())/2, sy+(cellH-r.height())/2-1);
                }
                statusBar()->showMessage(QStringLiteral("顺序表：删除 Step 2 - 前移中"));
            }else{
                seq.erase(pos);
                QVector<QString> finalShow(n-1);
                for(int i=0;i<pos;++i) finalShow[i] = arr[i];
                for(int i=pos;i<n-1;++i) finalShow[i] = arr[i+1];
                for (int i=0;i<finalShow.size();++i){
                    bool hi = (i>=pos && i<=n-2);
                    qreal x = startX + i*(cellW+gap), y = startY;
                    view->Scene()->addRect(QRectF(x,y,cellW,cellH), QPen(QColor("#5f6c7b"),2), QBrush(hi? QColor("#ffd166"): QColor("#e8eef9")));
                    auto* tItem = view->Scene()->addText(finalShow[i]); tItem->setDefaultTextColor(Qt::black);
                    auto r=tItem->boundingRect(); tItem->setPos(x+(cellW-r.width())/2, y+(cellH-r.height())/2-1);
                    auto* idx = view->Scene()->addText(QString::number(i));
                    idx->setDefaultTextColor(Qt::darkGray);
                    idx->setPos(x+cellW/2-6, y+cellH+6);
                }
                statusBar()->showMessage(QStringLiteral("顺序表：删除完成"));
                timer.setInterval(prevInterval);
            }
        });
    }
    timer.start();
}

void MainWindow::seqlistClear() {
    seq.clear(); drawSeqlist(seq); statusBar()->showMessage(QStringLiteral("顺序表：已清空"));
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

void MainWindow::linklistInsert() {
    int pos = linklistPosition->value();
    bool ok = false; int v = linklistValue->text().toInt(&ok);
    if(!ok) { statusBar()->showMessage(QStringLiteral("链表：请输入有效的值")); return; }

    int n = link.size();
    if (pos < 0) pos = 0; if (pos > n) pos = n;

    QVector<int> vals; vals.reserve(n);
    for (int i = 0; i < n; ++i) vals.push_back(link.get(i));

    const qreal y = 220, dx = 120, startX = 120;
    QVector<QPointF> centers; centers.reserve(n);
    for (int i = 0; i < n; ++i) centers.push_back(QPointF(startX + i*dx, y));

    QPointF startQ(startX + pos*dx, y + 120);
    QPointF targetQ(startX + pos*dx, y);
    const int prevIndex = pos - 1;
    const int succIndex = (pos < n) ? pos : -1;

    timer.stop(); steps.clear(); stepIndex = 0;

    auto drawBase = [=](int highlightIndex, int skipEdgeFrom){
        view->resetScene(); view->setTitle(QStringLiteral("单链表：插入演示"));
        for (int i = 0; i < vals.size(); ++i){
            bool hl = (i == highlightIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);
            if (i > 0 && (i - 1) != skipEdgeFrom){
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }
    };

    steps.push_back([=](){ drawBase(prevIndex, -1); view->addNode(startQ.x(), startQ.y(), QString::number(v), true); statusBar()->showMessage(QStringLiteral("链表：插入前")); });

    steps.push_back([=](){ drawBase(prevIndex, -1); view->addNode(startQ.x(), startQ.y(), QString::number(v), true);
        if (succIndex != -1){ view->addEdge(QPointF(startQ.x()+34, startQ.y()), QPointF(centers[succIndex].x()-34, y)); }
        view->setTitle(QStringLiteral("单链表：设置 q->next")); statusBar()->showMessage(QStringLiteral("链表：步骤1 q->next=succ")); });

    steps.push_back([=](){
        if (prevIndex >= 0){
            drawBase(prevIndex, prevIndex);
            view->addNode(startQ.x(), startQ.y(), QString::number(v), true);
            if (succIndex != -1){ view->addEdge(QPointF(startQ.x()+34, startQ.y()), QPointF(centers[succIndex].x()-34, y)); }
            view->addEdge(QPointF(centers[prevIndex].x()+34, y), QPointF(startQ.x()-34, startQ.y()));
            view->setTitle(QStringLiteral("单链表：设置 p->next"));
        }else{
            drawBase(-1, -1);
            view->addNode(startQ.x(), startQ.y(), QString::number(v), true);
            if (succIndex != -1){ view->addEdge(QPointF(startQ.x()+34, startQ.y()), QPointF(centers[succIndex].x()-34, y)); }
            view->setTitle(QStringLiteral("单链表：head = q"));
        }
        statusBar()->showMessage(QStringLiteral("链表：步骤2 连接前驱"));
    });

    const int frames = 10; const int oldInterval = timer.interval();
    for (int f=0; f<=frames; ++f){
        steps.push_back([=](){
            if (f==0) timer.setInterval(60);
            const qreal t = qreal(f)/frames;
            drawBase(prevIndex, prevIndex);
            QPointF pos(lerp(startQ.x(), targetQ.x(), t), lerp(startQ.y(), targetQ.y(), t));
            if (prevIndex >= 0) view->addEdge(QPointF(centers[prevIndex].x()+34, y), QPointF(pos.x()-34, pos.y()));
            if (succIndex != -1) view->addEdge(QPointF(pos.x()+34, pos.y()), QPointF(centers[succIndex].x()-34, y));
            view->addNode(pos.x(), pos.y(), QString::number(v), true);
            view->setTitle(QStringLiteral("单链表：结点移动到位"));
            if (f==frames) timer.setInterval(oldInterval);
        });
    }

    steps.push_back([=](){ link.insert(pos, v); drawLinklist(link); view->setTitle(QStringLiteral("单链表：插入完成")); statusBar()->showMessage(QStringLiteral("链表：insert(%1,%2)").arg(pos).arg(v)); });
    timer.start();
}

void MainWindow::linklistErase() {
    int pos = linklistPosition->value();
    int n = link.size();
    if (pos < 0 || pos >= n) { statusBar()->showMessage(QStringLiteral("链表：删除失败(位置越界)")); return; }

    QVector<int> vals; vals.reserve(n);
    for (int i = 0; i < n; ++i) vals.push_back(link.get(i));

    const qreal y = 220, dx = 120, startX = 120;
    QVector<QPointF> centers; centers.reserve(n);
    for (int i = 0; i < n; ++i) centers.push_back(QPointF(startX + i*dx, y));

    const int prevIndex = pos - 1, qIndex = pos, succIndex = (pos + 1 < n) ? pos + 1 : -1;

    timer.stop(); steps.clear(); stepIndex = 0;

    auto drawBase = [=](int highlightIndex, int skipEdgeFrom){
        view->resetScene(); view->setTitle(QStringLiteral("单链表：删除演示"));
        for (int i = 0; i < vals.size(); ++i){
            bool hl = (i == highlightIndex);
            QPointF c = centers[i];
            view->addNode(c.x(), y, QString::number(vals[i]), hl);
            auto* idxItem = view->Scene()->addText(QString::number(i));
            idxItem->setDefaultTextColor(Qt::darkGray);
            idxItem->setPos(c.x()-6, y+40);
            if (i > 0 && (i - 1) != skipEdgeFrom){
                view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
            }
        }
    };

    steps.push_back([=](){ drawBase(qIndex, -1); view->setTitle(QStringLiteral("单链表：删除前（高亮 q）")); statusBar()->showMessage(QStringLiteral("链表：删除前")); });

    steps.push_back([=](){
        drawBase(qIndex, prevIndex);
        if (prevIndex >= 0 && succIndex != -1) {
            QPointF ps(centers[prevIndex].x()+34, y), pe(centers[succIndex].x()-34, y);
            QPointF c1(ps.x()+40, ps.y()-120), c2(pe.x()-40, ps.y()-120);
            view->addCurveArrow(ps, c1, c2, pe);
        }
        view->setTitle(prevIndex>=0? QStringLiteral("单链表：p->next = q->next") : QStringLiteral("单链表：head = head->next"));
        statusBar()->showMessage(QStringLiteral("链表：步骤1 重新连接指针"));
    });

    const int frames = 10; const int oldInterval = timer.interval();
    QPointF qStart = centers[qIndex], qEnd = qStart + QPointF(0, 120);
    for (int f=0; f<=frames; ++f){
        steps.push_back([=](){
            if (f==0) timer.setInterval(60);
            const qreal t = qreal(f)/frames;
            view->resetScene();
            for (int i = 0; i < vals.size(); ++i){
                if (i==qIndex) continue;
                QPointF c = centers[i];
                view->addNode(c.x(), y, QString::number(vals[i]), false);
                if (i > 0 && i-1 != prevIndex && i!=qIndex){
                    view->addEdge(QPointF(centers[i-1].x()+34, y), QPointF(centers[i].x()-34, y));
                }
            }
            if (prevIndex >= 0 && succIndex != -1){
                view->addEdge(QPointF(centers[prevIndex].x()+34, y), QPointF(centers[succIndex].x()-34, y));
            }
            QPointF pos(lerp(qStart.x(), qEnd.x(), t), lerp(qStart.y(), qEnd.y(), t));
            view->addNode(pos.x(), pos.y(), QString::number(vals[qIndex]), true);
            view->setTitle(QStringLiteral("单链表：q 移出链表"));
            if (f==frames) timer.setInterval(oldInterval);
        });
    }

    steps.push_back([=](){ link.erase(pos); drawLinklist(link); view->setTitle(QStringLiteral("单链表：删除完成")); statusBar()->showMessage(QStringLiteral("链表：erase(%1)").arg(pos)); });
    timer.start();
}

void MainWindow::linklistClear() { link.clear(); drawLinklist(link); statusBar()->showMessage(QStringLiteral("链表：已清空")); }

// ===== 栈 =====
void MainWindow::stackBuild(){
    st.clear();
    auto a = parseIntList(stackInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene(); view->setTitle(QStringLiteral("栈：建立"));
    steps.push_back([this](){ drawStack(st); statusBar()->showMessage(QStringLiteral("栈：开始建立")); });
    for (int x : a){
        steps.push_back([this, x](){ st.push(x); drawStack(st); statusBar()->showMessage(QStringLiteral("栈：插入 %1").arg(x)); });
    }
    timer.start();
}

void MainWindow::stackPush() {
    bool ok = false; int v = stackValue->text().toInt(&ok);
    if(!ok) { statusBar()->showMessage(QStringLiteral("栈：请输入有效的值")); return; }

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

    steps.push_back([=](){ st.push(v); drawStack(st); view->setTitle(QStringLiteral("栈：入栈完成")); statusBar()->showMessage(QStringLiteral("栈：push(%1)").arg(v)); });
    timer.start();
}

void MainWindow::stackPop() {
    if(st.size()==0){ statusBar()->showMessage(QStringLiteral("栈：空栈，无法出栈")); return; }

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

    steps.push_back([this](){ int out=0; st.pop(&out); drawStack(st); view->setTitle(QStringLiteral("栈：出栈完成")); statusBar()->showMessage(QStringLiteral("栈：%1 出栈").arg(out)); });
    timer.start();
}

void MainWindow::stackClear() { st.clear(); drawStack(st); statusBar()->showMessage(QStringLiteral("栈：已清空")); }

// ===== 二叉树（构建 & 遍历逐帧高亮） =====
void MainWindow::btBuild(){
    auto a = parseIntList(btInput->text());
    int sent = btNull->value();
    timer.stop(); steps.clear(); stepIndex = 0;

    steps.push_back([this](){
        bt.clear(); view->resetScene(); view->setTitle(QStringLiteral("二叉树：开始建立（空树）"));
        drawBT(bt.root(), 400, 120, 200, 0); statusBar()->showMessage(QStringLiteral("二叉树：开始建立（空树）"));
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
            statusBar()->showMessage(QStringLiteral("二叉树：步骤 %1/%2，%3").arg(i+1).arg(a.size()).arg(msg));
        });
    }
    timer.start();
}

void MainWindow::btClear() {
    bt.clear(); view->resetScene(); view->setTitle(QStringLiteral("二叉树（空）")); statusBar()->showMessage(QStringLiteral("二叉树：已清空"));
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
    if(!ok) { statusBar()->showMessage(QStringLiteral("二叉搜索树：请输入有效的键值")); return; }

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
    timer.start();
}

void MainWindow::bstErase() {
    bool ok = false; int value = bstValue->text().toInt(&ok);
    if(!ok) { statusBar()->showMessage(QStringLiteral("二叉搜索树：请输入有效的键值")); return; }

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
    if (w.isEmpty()) { view->resetScene(); view->setTitle(QStringLiteral("哈夫曼树：请输入权值序列")); statusBar()->showMessage(QStringLiteral("哈夫曼树：无有效输入")); return; }
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
        statusBar()->showMessage(QStringLiteral("哈夫曼树：完成"));
    });

    timer.start();
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
    auto* p = ll.gethead(); qreal x=120, y=220, lastx=-1; int i = 0;
    while(p) {
        view->addNode(x, y, QString::number(ll.get(i)));
        auto* idx = view->Scene()->addText(QString::number(i)); idx->setDefaultTextColor(Qt::darkGray); idx->setPos(x-6, y+40);
        if(lastx > 0) view->addEdge(QPointF(lastx,y), QPointF(x-34,y));
        lastx = x+34; x += 120; p = p->next; i++;
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