//
// Created by xiang on 25-11-26.
//
#include "mainwindow.h"
#include "dsl.h"
#include "llmclient.h"

#include <QGraphicsScene>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimeLine>
#include <QTableWidget>
#include <QRegularExpression>
#include <QPointF>
#include <cmath>
#include <memory>

// 两个全局辅助变量和函数
static ds::BTNode* g_btHighlightNode = nullptr; //二叉树高亮节点
static qreal lerp(qreal a, qreal b, qreal t){ return a + (b - a) * t; } //给 AVL 旋转“丝滑动画”用，逐帧把结点坐标从 posBefore 过渡到 posAfter

// ===== 顺序表 =====
void MainWindow::seqlistBuild()
{
    auto a = parseIntList(seqlistInput->text());  //auto: 自动类型推导

    // 先停掉当前动画并清空步骤
    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 立即清空画布和标题（避免用户看到旧内容）
    view->resetScene();
    view->setTitle(QStringLiteral("顺序表：建立"));

    // 第 0 步：每次播放 / 重播时都从“空表”开始
    steps.push_back([this]() {
        seq.clear();  // 关键：重播时重新清空顺序表
        view->resetScene();
        view->setTitle(QStringLiteral("顺序表：建立"));
        drawSeqlist(seq);
        showMessage(QStringLiteral("顺序表：开始建立"));
    });

    // 后续步骤：逐个插入元素
    for (int x : a) {  //把 a 里面的每一个元素，挨个取出来，赋值给 x，执行后续操作。
        steps.push_back([this, x]() {
            seq.insert(seq.size(), x);
            drawSeqlist(seq);
            showMessage(QStringLiteral("顺序表：插入元素 %1").arg(x));
        });
    }

    // 自动开始播放
    timer.start();
    updateAnimUiState();//刷新按钮
}


void MainWindow::seqlistInsert(){
    view->setCurrentFamily(QStringLiteral("seq"));
    int pos = seqlistPosition->value();
    bool ok = false;
    int val = seqlistValue->text().toInt(&ok);
    if (!ok) {
        showMessage(QStringLiteral("顺序表：请输入有效的值"));
        return;
    }

    const int n = seq.size();
    if (pos < 0) pos = 0;
    if (pos > n) pos = n;

    // 先把当前顺序表的值拷出来，用于纯前端动画
    QVector<QString> arr(n);
    for (int i = 0; i < n; ++i)
        arr[i] = QString::number(seq.get(i));

    const qreal cellW = 68;
    const qreal cellH = 54;
    const qreal gap = 14;
    const qreal startX = 80;
    const qreal startY = 180;

    const int animInterval = qMax(15, timer.interval() / 7);//记录当前动画计时器间隔

    // 根据尾部长度控制每个“小动画”的帧数，避免元素过多时太慢
    int tail = n - pos;
    int framesShift = 10;
    if (tail > 80) framesShift = 6;  //计算“插入位置右侧有多少元素需要右移”，并据此降低每个元素移动的帧数
    if (tail > 200) framesShift = 4;

    const int framesDrop = 10;  //新元素“从上掉落到目标位置”的帧数

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 步骤 0：显示当前状态，高亮插入位置
    steps.push_back([=, this]() {
    // ★ 重播关键：每次播放前先把顺序表还原到“插入前”的状态
    seq.clear();
    for (int i = 0; i < n; ++i) {
        seq.insert(i, arr[i].toInt());
    }

    view->resetScene();
    view->setTitle(QStringLiteral("顺序表：插入前（pos=%1）").arg(pos));

    for (int i = 0; i < n; ++i) { //逐个画出 n 个格子
        qreal x = startX + i * (cellW + gap);
        qreal y = startY;
        view->Scene()->addRect(QRectF(x, y, cellW, cellH), QPen(QColor("#5f6c7b"), 2), QBrush(QColor("#e8eef9")));
        auto *t = view->Scene()->addText(arr[i]);
        t->setDefaultTextColor(Qt::black);
        QRectF tb = t->boundingRect();
        t->setPos(x + (cellW - tb.width()) / 2, y + (cellH - tb.height()) / 2 - 1);

        auto *idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(Qt::darkGray);
        idx->setPos(x + cellW / 2 - 6, y + cellH + 6);
    }

    // 高亮插入位置
    qreal hx = startX + pos * (cellW + gap);
    view->Scene()->addRect(QRectF(hx, startY, cellW, cellH), QPen(QColor("#ef4444"), 3), QBrush(Qt::transparent));
    showMessage(QStringLiteral("顺序表：准备在位置 %1 插入").arg(pos));
    });

    // 步骤 1：把计时器调成动画间隔
    steps.push_back([=, this]() {
        timer.setInterval(animInterval);
    });

    // 步骤 2：从右往左，一个一个挪动元素 i -> i+1
    // k 表示当前在移动的元素下标
    for (int k = n - 1; k >= pos; --k) {  //从最右端开始移动，依次移动 k 位置的元素到 k+1
        for (int f = 0; f <= framesShift; ++f) {  //每个元素的移动由多帧组成，实现平滑移动
            const qreal t = (framesShift == 0) ? 1.0 : (qreal)f / framesShift;  //t:当前移动的元素的动画进度百分比

            steps.push_back([=, this]() {
                view->resetScene();
                view->setTitle(QStringLiteral("顺序表：移动元素 %1 → %2") .arg(k).arg(k + 1));

                for (int j = 0; j < n; ++j) {
                    // 计算“当前这一帧里，这个元素显示在第几个格子上”
                    qreal idxPos;  // 可能是小数，用来做插值

                    if (j < pos) {  // 插入位置之前的元素不动
                        idxPos = j;
                    } else if (j > k) {  // 已经完成移动的右边部分：全部在最终位置 j+1
                        idxPos = j + 1;
                    } else if (j == k) {  // 当前要移动的这个元素：从 k 挪到 k+1
                        idxPos = (qreal)k + t;
                    } else {// pos <= j < k：还没轮到移动，保持在位置 j
                        idxPos = j;
                    }

                    qreal x = startX + idxPos * (cellW + gap);
                    qreal y = startY;

                    bool highlight = (j == k);  //是否是当前插入的元素

                    view->addBox(x, y, cellW, cellH, arr[j], highlight);

                    auto *tItem = view->Scene()->addText(arr[j]);  //逐个逐帧写值
                    tItem->setDefaultTextColor(Qt::black);
                    QRectF tb = tItem->boundingRect();
                    tItem->setPos(x + (cellW - tb.width()) / 2, y + (cellH - tb.height()) / 2 - 1);
                }

                // 在整个移动过程中，新元素先停在插入位置上方，不参与移动
                qreal newX = startX + pos * (cellW + gap);
                qreal newY = startY - 80;
                view->Scene()->addRect(QRectF(newX, newY, cellW, cellH), QPen(QColor("#22c55e"), 2), QBrush(QColor("#bbf7d0")));
                auto *newText = view->Scene()->addText(QString::number(val));
                newText->setDefaultTextColor(Qt::black);
                QRectF tbNew = newText->boundingRect();
                newText->setPos(newX + (cellW - tbNew.width()) / 2, newY + (cellH - tbNew.height()) / 2 - 1);
            });
        }
    }

    // 步骤 3：让新元素从上往下“掉”到 pos 位置
    for (int f = 0; f <= framesDrop; ++f) {
        const qreal t = (framesDrop == 0) ? 1.0 : (qreal)f / framesDrop;  //t:当前移动的元素的动画进度百分比

        steps.push_back([=, this]() {
            view->resetScene();
            view->setTitle(QStringLiteral("顺序表：插入新元素"));

            // 先画已经“挪完”的老元素（全都在最终位置了）
            const int m = n + 1;
            for (int i = 0; i < m; ++i) {
                qreal x = startX + i * (cellW + gap);
                qreal y = startY;

                QBrush boxBrush(QColor("#e8eef9"));
                QString text;

                if (i < pos) text = arr[i];
                else if (i == pos) {
                    // 留空的方框，让上面掉下来的块来填
                    boxBrush = QBrush(Qt::NoBrush);
                } else {
                    text = arr[i - 1];
                }

                view->Scene()->addRect(QRectF(x, y, cellW, cellH), QPen(QColor("#5f6c7b"), 2), boxBrush);
                if (!text.isEmpty()) {
                    auto *tItem = view->Scene()->addText(text);
                    tItem->setDefaultTextColor(Qt::black);
                    QRectF tb = tItem->boundingRect();
                    tItem->setPos(x + (cellW - tb.width()) / 2, y + (cellH - tb.height()) / 2 - 1);
                }
            }

            // 画“正在下落”的新元素
            qreal baseX = startX + pos * (cellW + gap);
            qreal startYTop = startY - 80;
            qreal curY = startYTop + (startY - startYTop) * t;

            view->Scene()->addRect(QRectF(baseX, curY, cellW, cellH), QPen(QColor("#22c55e"), 2), QBrush(QColor("#bbf7d0")));
            auto *newText = view->Scene()->addText(QString::number(val));
            newText->setDefaultTextColor(Qt::black);
            QRectF tbNew = newText->boundingRect();
            newText->setPos(baseX + (cellW - tbNew.width()) / 2, curY + (cellH - tbNew.height()) / 2 - 1);
        });
    }

    // 步骤 4：真正往后端顺序表里插入 + 恢复计时器 + 画最终结果
    steps.push_back([=, this]() {
        onAnimSpeedChanged(animSpeedSlider->value());
        seq.insert(pos, val);
        view->resetScene();
        view->setTitle(QStringLiteral("顺序表：插入完成（pos=%1, val=%2）") .arg(pos).arg(val));
        drawSeqlist(seq);
        showMessage(QStringLiteral("顺序表：插入完成"));
    });

    timer.start();
}

void MainWindow::seqlistErase(){
    view->setCurrentFamily(QStringLiteral("seq"));
    int pos = seqlistPosition->value();
    const int n = seq.size();
    if (pos < 0 || pos >= n) {
        showMessage(QStringLiteral("顺序表：删除失败(位置越界)"));
        return;
    }

    QVector<QString> arr(n);
    for (int i = 0; i < n; ++i)
        arr[i] = QString::number(seq.get(i));

    const qreal cellW  = 68;
    const qreal cellH  = 54;
    const qreal gap    = 14;
    const qreal startX = 80;
    const qreal startY = 180;

    const int animInterval = qMax(15, timer.interval() / 7);

    int tail = n - 1 - pos;
    int framesShift = 10;
    if (tail > 80)  framesShift = 6;
    if (tail > 200) framesShift = 4;

    const int framesDelete = 10;

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 步骤 0：显示当前状态，高亮要删除的格子
    steps.push_back([=, this]() {
    seq.clear();
    for (int i = 0; i < n; ++i) {
        seq.insert(i, arr[i].toInt());
    }

    view->resetScene();
    view->setTitle(QStringLiteral("顺序表：删除前（pos=%1）").arg(pos));

    QBrush normalBrush(QColor("#e5f3ff"));
    QBrush delBrush(QColor("#fecaca"));

    for (int i = 0; i < n; ++i) {
        bool hi = (i == pos);
        qreal x = startX + i * (cellW + gap);
        qreal y = startY;
        view->Scene()->addRect(QRectF(x, y, cellW, cellH), QPen(QColor("#5f6c7b"), 2), QBrush(hi ? QColor("#fecaca") : QColor("#e8eef9")));
        auto *t = view->Scene()->addText(arr[i]);
        t->setDefaultTextColor(Qt::black);
        QRectF tb = t->boundingRect();
        t->setPos(x + (cellW - tb.width()) / 2, y + (cellH - tb.height()) / 2 - 1);

        auto *idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(Qt::darkGray);
        idx->setPos(x + cellW / 2 - 6, y + cellH + 6);
    }

    showMessage(QStringLiteral("顺序表：当前长度 %1，准备删除位置 %2 的元素").arg(n).arg(pos));
    timer.setInterval(animInterval);
    });

    // 步骤 1：把计时器调成动画间隔
    steps.push_back([=, this]() {
        timer.setInterval(animInterval);
    });

    // 步骤 2：把要删除的那个格子“抬上去 + 变透明”
    for (int f = 0; f <= framesDelete; ++f) {
        const qreal t = (framesDelete == 0) ? 1.0 : (qreal)f / framesDelete;  //t:当前移动的元素的动画进度百分比

        steps.push_back([=, this]() {
            view->resetScene();
            view->setTitle(QStringLiteral("顺序表：删除元素"));

            // 其它格子原地不动（不画 pos）
            for (int i = 0; i < n; ++i) {
                if (i == pos) continue;
                qreal x = startX + i * (cellW + gap);
                qreal y = startY;
                view->Scene()->addRect(QRectF(x, y, cellW, cellH), QPen(QColor("#5f6c7b"), 2), QBrush(QColor("#e8eef9")));
                auto *tItem = view->Scene()->addText(arr[i]);
                tItem->setDefaultTextColor(Qt::black);
                QRectF tb = tItem->boundingRect();
                tItem->setPos(x + (cellW - tb.width()) / 2, y + (cellH - tb.height()) / 2 - 1);
            }

            // 被删的元素向上抬起并逐渐透明
            qreal baseX = startX + pos * (cellW + gap);
            qreal baseY = startY;
            qreal curY  = baseY - 80 * t;
            qreal alpha = 1.0 - t;

            auto *rect = view->Scene()->addRect(QRectF(baseX, curY, cellW, cellH), QPen(QColor("#ef4444"), 2), QBrush(QColor("#fecaca")));
            rect->setOpacity(alpha);

            auto *text = view->Scene()->addText(arr[pos]);
            text->setDefaultTextColor(Qt::black);
            text->setOpacity(alpha);
            QRectF tb = text->boundingRect();
            text->setPos(baseX + (cellW - tb.width()) / 2, curY + (cellH - tb.height()) / 2 - 1);
        });
    }

    // 步骤 3：从左到右，一个一个把后面的元素往前挪：k: pos+1 -> n-1
    for (int k = pos + 1; k < n; ++k) {
        for (int f = 0; f <= framesShift; ++f) {
            const qreal t = (framesShift == 0) ? 1.0 : (qreal)f / framesShift;

            steps.push_back([=, this]() {
                view->resetScene();
                view->setTitle(QStringLiteral("顺序表：移动元素 %1 → %2").arg(k).arg(k - 1));

                for (int j = 0; j < n; ++j) {
                    if (j == pos) continue; // 被删的元素不再画

                    // 计算这一帧中 arr[j] 所在的格子索引（可能是小数）
                    qreal idxPos;
                    if (j < pos) {  // 删除位置之前的元素不动
                        idxPos = j;
                    } else if (j > k) {  // 还没轮到移动的尾部元素：一直保持原位 j
                        idxPos = j;
                    } else if (j == k) {  // 当前正在移动的元素：从 k → k-1
                        idxPos = (qreal)k - t;
                    } else {  // pos < j < k：已经在之前的步骤中移动过一次，停在 j-1
                        idxPos = j - 1;
                    }

                    //开始绘制所有不需要变的格子
                    qreal x = startX + idxPos * (cellW + gap);
                    qreal y = startY;

                    bool highlight = (j == k);
                    view->addBox(x, y, cellW, cellH, arr[j], highlight);

                    auto *tItem = view->Scene()->addText(arr[j]);
                    tItem->setDefaultTextColor(Qt::black);
                    QRectF tb = tItem->boundingRect();
                    tItem->setPos(x + (cellW - tb.width()) / 2, y + (cellH - tb.height()) / 2 - 1);
                }
            });
        }
    }

    // 步骤 4：真正删除后端元素 + 恢复计时器 + 画最终顺序表
    steps.push_back([=, this]() {
        onAnimSpeedChanged(animSpeedSlider->value());
        seq.erase(pos);
        view->resetScene();
        view->setTitle(QStringLiteral("顺序表：删除完成（pos=%1）").arg(pos));
        drawSeqlist(seq);
        showMessage(QStringLiteral("顺序表：删除完成"));
    });

    timer.start();
}

void MainWindow::seqlistClear() {
    seq.clear(); drawSeqlist(seq); showMessage(QStringLiteral("顺序表：已清空"));
}

// ===== 链表 =====
void MainWindow::linklistBuild()
{
    auto a = parseIntList(linklistInput->text());

    timer.stop();
    steps.clear();
    stepIndex = 0;

    view->resetScene();
    view->setTitle(QStringLiteral("链表：建立"));

    // 第 0 步：从空链表开始（重播时也会执行）
    steps.push_back([this]() {
        link.clear();
        view->resetScene();
        view->setTitle(QStringLiteral("链表：建立"));
        drawLinklist(link);
        showMessage(QStringLiteral("链表：开始建立"));
    });

    for (int x : a) {
        steps.push_back([this, x]() {
            link.insert(link.size(), x);
            drawLinklist(link);
            showMessage(QStringLiteral("链表：插入元素 %1").arg(x));
        });
    }

    timer.start();
    updateAnimUiState();  //更新按钮状态
}

void MainWindow::linklistInsert() {
    int pos = linklistPosition->value();
    bool ok = false; int v = linklistValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("链表：请输入有效的值")); return; }

    int n = link.size();
    if (pos < 0) pos = 0; if (pos > n) pos = n;

    //把链表当前内容抽出来变成顺序数组
    QVector<int> vals; vals.reserve(n);
    for (int i = 0; i < n; ++i) vals.push_back(link.get(i));

    //计算绘制布局
    const qreal y = 220, dx = 120, startX = 150; // 增加startX，让链表整体右移
    QVector<QPointF> centers; centers.reserve(n);
    for (int i = 0; i < n; ++i) centers.push_back(QPointF(startX + i*dx, y));

    const int prevIndex = pos - 1;//前驱节点
    const int succIndex = (pos < n) ? pos : -1;//后继节点

    timer.stop(); steps.clear(); stepIndex = 0;

    // 步骤1：显示当前链表状态，高亮相关节点
    steps.push_back([=, this]() {
        link.clear();
        for (int x : vals) {
            link.insert(link.size(), x);   // 相当于 push_back
        }
        view->resetScene(); view->setTitle(QStringLiteral("单链表：插入前"));
        QBrush normalBrush(QColor("#e5f3ff"));
        QBrush highlightBrushPrev(QColor("#fbbf24"));
        QBrush highlightBrushSucc(QColor("#22c55e"));

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

    steps.push_back([=, this]() {
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
    steps.push_back([=, this]() {
        view->resetScene(); view->setTitle(QStringLiteral("单链表：执行 q->next = p->next"));

        // 绘制原有链表
        for (int i = 0; i < vals.size(); ++i) {
            bool hl = (i == prevIndex || (succIndex != -1 && i == succIndex));//前驱后继节点高亮
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

        // 绘制 q->next = p->next 的连接
        if (succIndex != -1) {
            // 使用直线连接，避免过度弯曲
            QPointF start(qPos.x()+34, qPos.y());
            QPointF end(centers[succIndex].x()-34, y);

            // 计算控制点，使曲线更平缓
            qreal controlOffset = 40; // 较小的控制点偏移
            QPointF c1(start.x() + controlOffset, start.y());
            QPointF c2(end.x() - controlOffset, end.y());

            view->addCurveArrow(start, c1, c2, end);//绘制箭头

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
    steps.push_back([=, this]() {
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

    // 绘制 p->next = q 的连接
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
    steps.push_back([=, this]() {
        timer.setInterval(fastInterval);
        showMessage(QStringLiteral("快速调整节点布局..."));
    });

    for (int f = 0; f <= moveFrames; ++f) {
        steps.push_back([=, this]() {
            qreal t = (qreal)f / moveFrames;
            // 使用缓动函数使动画更自然
            qreal easedT = 1 - std::pow(1 - t, 2); // 缓出效果

            QPointF currentPos = qPos + (finalPos - qPos) * easedT;//新节点当前动画的位置

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
    steps.push_back([=, this]() {
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
    steps.push_back([=, this]() {
        link.clear();
        for (int x : vals) {
            link.insert(link.size(), x);
        }

        view->resetScene();
        view->setTitle(QStringLiteral("单链表：删除前"));

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
    steps.push_back([=, this]() {
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
    steps.push_back([=, this]() {
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
    steps.push_back([=, this]() {
        timer.setInterval(fastInterval);
        showMessage(QStringLiteral("快速删除节点..."));
    });

    for (int f = 0; f <= deleteFrames; ++f) {
    steps.push_back([=, this]() {
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
    steps.push_back([=, this]() {
        timer.setInterval(fastInterval);
        showMessage(QStringLiteral("快速调整布局..."));
    });

    for (int f = 0; f <= adjustFrames; ++f) {
        steps.push_back([=, this]() {
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
    steps.push_back([=, this]() {
        link.erase(pos);
        drawLinklist(link);
        view->setTitle(QStringLiteral("单链表：删除完成"));
        showMessage(QStringLiteral("链表：删除位置 %1 完成").arg(pos));
    });

    timer.start();
}

void MainWindow::linklistClear() { link.clear(); drawLinklist(link); statusBar()->showMessage(QStringLiteral("链表：已清空")); }

// ===== 栈 =====
void MainWindow::stackBuild()
{
    auto a = parseIntList(stackInput->text());
    timer.stop(); steps.clear(); stepIndex = 0;
    view->resetScene(); view->setTitle(QStringLiteral("顺序栈：建立"));

    // 第 0 步：从空栈开始（重播时也会执行）
    steps.push_back([this]() {
        st.clear();
        view->resetScene();
        view->setTitle(QStringLiteral("顺序栈：建立"));
        drawStack(st);
        showMessage(QStringLiteral("顺序栈：开始建立"));
    });

    for (int x : a) {
        steps.push_back([this, x]() {
            st.push(x);
            drawStack(st);
            showMessage(QStringLiteral("顺序栈：入栈元素 %1").arg(x));
        });
    }

    timer.start();
    updateAnimUiState();
}

void MainWindow::stackPush() {
    view->setCurrentFamily(QStringLiteral("stack"));
    bool ok = false; int v = stackValue->text().toInt(&ok);
    if(!ok) { showMessage(QStringLiteral("栈：请输入有效的值")); return; }

    const qreal x0 = 380, y0 = 120, W = 300, T = 12, innerPad = 6;// x0,y0: 栈槽(U形)左上角；W: 槽宽；T: 槽壁厚；innerPad: 槽内左右留白
    const qreal BLOCK_H = 32, GAP = 4;// BLOCK_H: 每个元素块高度；GAP: 元素块之间的竖向间距
    const int n = st.size();// 当前栈内元素数量（入栈前的 size）
    const int levels = std::max(n, 6);// 为了“槽高度自适应且至少能装下6层”，取 max(n,6) 作为显示层数
    const qreal innerH = levels * BLOCK_H + (levels - 1) * GAP;// 槽内部可用高度：levels 个块 + (levels-1) 个间隙
    const qreal H = innerH + T + BLOCK_H;// 整个槽的总高度：内部高度 + 底部槽壁厚T + 预留一个块高度（方便顶部动画/空间）
    const qreal innerW = W - 2*T - 2*innerPad;// 槽内部可用宽度：总宽W扣掉两侧槽壁厚与两侧内边距
    const qreal leftX = x0 + T + innerPad;// 内部区域左边界X：从槽左边x0向右跨过槽壁厚T和内边距innerPad
    const qreal bottomInnerY = y0 + H - T;// 槽内部“底部”Y（不含底部槽壁）：总高度到底再往上扣掉底壁厚T
    const qreal yTopBlock = bottomInnerY - (n+1) * BLOCK_H - (n) * GAP;// 新入栈元素最终应落到的位置(作为新的栈顶)：在底部之上放 (n+1) 个块、n 个间隙
    const qreal xCenter = leftX + innerW/2;// 槽内部水平中心X：用于把文字/块居中对齐
    const qreal yStart = yTopBlock - 80;// 新元素动画起始Y：比最终落点再往上80像素（从上方“掉落/下落”进入）

    // ★ 新增：保存“入栈前”的栈内容
    QVector<int> vals; // vals：用于保存旧栈快照（支持重播时还原）
    vals.reserve(n);// 预分配容量，避免多次扩容
    for (int i = 0; i < n; ++i) {
        vals.push_back(st.get(i));
    }

    const int frames = 10;
    const int animInterval = qMax(15, timer.interval() / 7);
    timer.stop(); steps.clear(); stepIndex = 0;

    for (int f=0; f<=frames; ++f){
        steps.push_back([=, this](){
            if (f==0) {
                // ★ 重播关键：每次从头播放时先还原栈
                st.clear();
                for (int x : vals) st.push(x);
                timer.setInterval(animInterval);
            }
            const qreal t = qreal(f)/frames;
            view->resetScene(); view->setTitle(QStringLiteral("栈：入栈（移动中）"));
            QGraphicsScene* S = view->Scene();
            QBrush wall(QColor("#334155")); QPen none(Qt::NoPen);
            S->addRect(QRectF(x0, y0, T, H), none, wall);// 画左侧槽壁
            S->addRect(QRectF(x0+W-T, y0, T, H), none, wall); // 画右侧槽壁
            S->addRect(QRectF(x0, y0+H-T, W, T), none, wall);// 画底部槽壁
            QPen boxPen(QColor("#1e293b")); boxPen.setWidthF(1.2);
            QBrush fill(QColor("#93c5fd")), topFill(QColor("#60a5fa"));// fill：普通元素填充色；topFill：栈顶高亮色
            const int nNow = st.size();
            for (int i = 0; i < nNow; ++i) {
                const bool isTop = (i == nNow - 1);
                const qreal yTop = bottomInnerY - (i + 1) * BLOCK_H - i * GAP;
                view->addBox(leftX, yTop, innerW, BLOCK_H, QString::number(st.get(i)), isTop);
            }

            // 新入栈块下落（视为高亮）
            qreal yTop = lerp(yStart, yTopBlock, t);
            view->addBox(leftX, yTop, innerW, BLOCK_H, QString::number(v), true);
            auto* label = S->addText(QString::number(v)); label->setDefaultTextColor(Qt::black);
            QRectF tb = label->boundingRect(); label->setPos(xCenter - tb.width()/2, yTop + BLOCK_H/2 - tb.height()/2);
            if (f==frames) onAnimSpeedChanged(animSpeedSlider->value());
        });
    }

    steps.push_back([=, this]() {
        st.push(v);
        drawStack(st);
        view->setTitle(QStringLiteral("栈：入栈完成"));
        showMessage(QStringLiteral("栈：push(%1)").arg(v));
    });
    timer.start();
}

void MainWindow::stackPop() {
    if(st.size()==0){ showMessage(QStringLiteral("栈：空栈，无法出栈")); return; }

    const qreal x0 = 380, y0 = 120, W = 300, T = 12, innerPad = 6;
    const qreal BLOCK_H = 32, GAP = 4;
    const int n = st.size();
    const int levels = std::max(n, 6);
    const qreal innerH = levels * BLOCK_H + (levels - 1) * GAP;
    const qreal H = innerH + T + BLOCK_H;
    const qreal innerW = W - 2*T - 2*innerPad;
    const qreal leftX = x0 + T + innerPad;
    const qreal bottomInnerY = y0 + H - T;
    const qreal xCenter = leftX + innerW/2;

    int topVal = st.getPeek();
    const qreal yTopBlock = bottomInnerY - n * BLOCK_H - (n - 1) * GAP;
    const qreal yEnd = yTopBlock - 80;

    // 保存“出栈前”的栈内容
    QVector<int> vals;
    vals.reserve(n);
    for (int i = 0; i < n; ++i) {
        vals.push_back(st.get(i));
    }

    const int frames = 10;
    const int animInterval = qMax(15, timer.interval() / 7);

    timer.stop(); steps.clear(); stepIndex = 0;

    for (int f=0; f<=frames; ++f){
        steps.push_back([=, this](){
            if (f == 0) {
                // ★ 重播关键：每次从头播放时先恢复栈
                st.clear();
                for (int x : vals) st.push(x);
                timer.setInterval(animInterval);
            }
            const qreal t = qreal(f)/frames;
            view->resetScene(); view->setTitle(QStringLiteral("栈：出栈（移动中）"));

            QGraphicsScene* S = view->Scene();
            QBrush wall(QColor("#334155")); QPen none(Qt::NoPen);
            S->addRect(QRectF(x0, y0, T, H), none, wall);
            S->addRect(QRectF(x0+W-T, y0, T, H), none, wall);
            S->addRect(QRectF(x0, y0+H-T, W, T), none, wall);

            QPen boxPen(QColor("#1e293b")); boxPen.setWidthF(1.2);
            QBrush fill(QColor("#93c5fd")), topFill(QColor("#60a5fa"));
            for (int i = 0; i < n - 1; ++i) {
                const bool isTop = (i == n - 2);
                const qreal yTop = bottomInnerY - (i + 1) * BLOCK_H - i * GAP;
                view->addBox(leftX, yTop, innerW, BLOCK_H, QString::number(st.get(i)), isTop);
            }

            qreal yTop = lerp(yTopBlock, yEnd, t);
            view->addBox(leftX, yTop, innerW, BLOCK_H, QString::number(topVal), true);
            auto* label = S->addText(QString::number(topVal)); label->setDefaultTextColor(Qt::black);
            QRectF tb = label->boundingRect(); label->setPos(xCenter - tb.width()/2, yTop + BLOCK_H/2 - tb.height()/2);

            if (f==frames) onAnimSpeedChanged(animSpeedSlider->value());
        });
    }

    steps.push_back([this]() {
        int out=0;
        st.pop(&out);
        drawStack(st);
        view->setTitle(QStringLiteral("栈：出栈完成"));
        showMessage(QStringLiteral("栈：%1 出栈").arg(out));
    });
    timer.start();
}

void MainWindow::stackClear() { st.clear(); drawStack(st); showMessage(QStringLiteral("栈：已清空")); }

// ===== 二叉树（构建 & 遍历逐帧高亮） =====
void MainWindow::btBuild(){
    auto a = parseIntList(btInput->text());
    int sent = btNull->value();//哨兵值
    timer.stop(); steps.clear(); stepIndex = 0;

    steps.push_back([this](){
        bt.clear(); view->resetScene(); view->setTitle(QStringLiteral("二叉树：开始建立（空树）"));
        drawBT(bt.root(), 400, 120, 200, 0); showMessage(QStringLiteral("二叉树：开始建立（空树）"));
    });

    for (int i = 0; i < a.size(); ++i){
        steps.push_back([=, this](){
            QVector<int> b(a.size(), sent);//创建一个长度与 a 相同的临时数组 b，并全部填充为哨兵 sent。目的：让“还没播放到的那些位置”先视为空位。
            for (int k = 0; k <= i; k++) b[k] = a[k];//把已经画过的节点拷贝过来
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
    int need = bt.preorder(nullptr, 0);//先探测遍历输出长度：传空指针和 0 容量，让后端只返回需要多少个元素
    if (need <= 0) {
        view->resetScene();
        view->setTitle(QStringLiteral("先序遍历：空树"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("先序遍历：空树"));
        return;
    }

    // 1) 后端拿“值序列”，用于文字提示
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.preorder(buf.get(), need);

    // 2) 前端再跑一遍先序，拿“结点指针序列”，因为动画需要用指针而不是节点序列
    QVector<ds::BTNode*> nodeOrder;
    nodeOrder.reserve(need);
    std::function<void(ds::BTNode*)> dfs = [&](ds::BTNode* p){
        if (!p) return;
        nodeOrder.push_back(p);
        dfs(p->left);
        dfs(p->right);
    };
    dfs(bt.root());

    int m = qMin(n, nodeOrder.size());

    timer.stop();
    steps.clear();
    stepIndex = 0;

    for (int i = 0; i < m; ++i) {
        int key = buf[i];
        ds::BTNode* node = nodeOrder[i];

        steps.push_back([this, i, m, key, node]() {
            view->resetScene();
            view->setTitle(QStringLiteral("先序遍历：访问 %1（%2/%3）").arg(key).arg(i + 1).arg(m));

            // 告诉 drawBT：这一次只高亮这个结点
            g_btHighlightNode = node;
            drawBT(bt.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("先序遍历：访问 %1").arg(key));
        });
    }

    timer.start();
}

void MainWindow::btInorder() {
    int need = bt.inorder(nullptr, 0);//先探测遍历输出长度：传空指针和 0 容量，让后端只返回需要多少个元素
    if (need <= 0) {
        view->resetScene();
        view->setTitle(QStringLiteral("中序遍历：空树"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("中序遍历：空树"));
        return;
    }

    // 1) 值序列
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.inorder(buf.get(), need);

    // 2) 结点指针序列（中序）
    QVector<ds::BTNode*> nodeOrder;
    nodeOrder.reserve(need);
    std::function<void(ds::BTNode*)> dfs = [&](ds::BTNode* p){
        if (!p) return;
        dfs(p->left);
        nodeOrder.push_back(p);
        dfs(p->right);
    };
    dfs(bt.root());

    int m = qMin(n, nodeOrder.size());

    timer.stop();
    steps.clear();
    stepIndex = 0;

    for (int i = 0; i < m; ++i) {
        int key = buf[i];
        ds::BTNode* node = nodeOrder[i];

        steps.push_back([this, i, m, key, node]() {
            view->resetScene();
            view->setTitle(QStringLiteral("中序遍历：访问 %1（%2/%3）").arg(key).arg(i + 1).arg(m));

            g_btHighlightNode = node;
            drawBT(bt.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("中序遍历：访问 %1").arg(key));
        });
    }

    timer.start();
}

void MainWindow::btPostorder() {
    int need = bt.postorder(nullptr, 0);//先探测遍历输出长度：传空指针和 0 容量，让后端只返回需要多少个元素
    if (need <= 0) {
        view->resetScene();
        view->setTitle(QStringLiteral("后序遍历：空树"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("后序遍历：空树"));
        return;
    }

    // 1) 值序列
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.postorder(buf.get(), need);

    // 2) 结点指针序列（后序）
    QVector<ds::BTNode*> nodeOrder;
    nodeOrder.reserve(need);
    std::function<void(ds::BTNode*)> dfs = [&](ds::BTNode* p){
        if (!p) return;
        dfs(p->left);
        dfs(p->right);
        nodeOrder.push_back(p);
    };
    dfs(bt.root());

    int m = qMin(n, nodeOrder.size());

    timer.stop();
    steps.clear();
    stepIndex = 0;

    for (int i = 0; i < m; ++i) {
        int key = buf[i];
        ds::BTNode* node = nodeOrder[i];

        steps.push_back([this, i, m, key, node]() {
            view->resetScene();
            view->setTitle(QStringLiteral("后序遍历：访问 %1（%2/%3）").arg(key).arg(i + 1).arg(m));

            g_btHighlightNode = node;
            drawBT(bt.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("后序遍历：访问 %1").arg(key));
        });
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

    int need = bt.levelorder(nullptr, 0);//先探测遍历输出长度：传空指针和 0 容量，让后端只返回需要多少个元素
    if (need <= 0) {
        view->resetScene();
        view->setTitle(QStringLiteral("层序遍历：空树"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("层序遍历：空树"));
        return;
    }

    // 1) 值序列
    std::unique_ptr<int[]> buf(new int[need]);
    int n = bt.levelorder(buf.get(), need);

    // 2) 结点指针序列（层序：队列）
    QVector<ds::BTNode*> nodeOrder;
    nodeOrder.reserve(need);
    QVector<ds::BTNode*> q;
    q.reserve(need);
    if (bt.root())
        q.push_back(bt.root());
    int head = 0;
    while (head < q.size()) {
        ds::BTNode* p = q[head++];
        nodeOrder.push_back(p);
        if (p->left)  q.push_back(p->left);
        if (p->right) q.push_back(p->right);
    }

    int m = qMin(n, nodeOrder.size());

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 起始状态
    steps.push_back([this]() {
        view->resetScene();
        view->setTitle(QStringLiteral("层序遍历：开始"));
        drawBT(bt.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("层序遍历：开始"));
    });

    // 每一步访问一个结点
    for (int i = 0; i < m; ++i) {
        int key = buf[i];
        ds::BTNode* node = nodeOrder[i];

        steps.push_back([this, i, m, key, node]() {
            view->resetScene();
            view->setTitle(QStringLiteral("层序遍历：访问 %1（%2/%3）").arg(key).arg(i + 1).arg(m));

            g_btHighlightNode = node;
            drawBT(bt.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("层序遍历：访问 %1").arg(key));
        });
    }

    // 结束状态
    steps.push_back([this]() {
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
        steps.push_back([=, this]() {
            bst.insert(a[i]);
            view->resetScene();
            view->setTitle(QStringLiteral("二叉搜索树：插入 %1（第 %2/%3 步）").arg(a[i]).arg(i+1).arg(a.size()));
            drawBT(bst.root(), 400, 120, 200, 0);
        });
    }
    timer.start();
}

void MainWindow::bstFind() {
    bool ok = false;
    int value = bstValue->text().toInt(&ok);
    if (!ok) {
        showMessage(QStringLiteral("二叉搜索树：请输入有效的键值"));
        return;
    }

    // 记录查找路径（指针）
    QVector<ds::BTNode*> path;
    ds::BTNode* p = bst.root();
    while (p) {
        path.push_back(p);
        if (value < p->key) {
            p = p->left;
        } else if (value > p->key) {
            p = p->right;
        } else break;
    }
    bool found = (!path.isEmpty() && path.last()->key == value);

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 高亮路径每一个节点
    for (int i = 0; i < path.size(); ++i) {
        ds::BTNode* node = path[i];
        steps.push_back([this, i, path, node, value, found]() {
            view->resetScene();
            view->setTitle(QStringLiteral("BST 查找 %1（%2/%3）").arg(value).arg(i+1).arg(path.size()));

            g_btHighlightNode = node;      // 按指针高亮
            drawBT(bst.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("BST 查找：比较 %1 和 %2").arg(node->key).arg(value));
        });
    }

    // 最后一帧：结果展示 + 弹窗提示
    steps.push_back([this, value, found]() {
        view->resetScene();
        if (found) {
            g_btHighlightNode = bst.find(value);
        }
        drawBT(bst.root(), 400, 120, 200, 0);
        g_btHighlightNode = nullptr;

        view->setTitle(QStringLiteral("BST 查找 %1：%2").arg(value).arg(found?QStringLiteral("找到"):QStringLiteral("未找到")));
        showMessage(found ? QStringLiteral("查找成功") : QStringLiteral("查找失败"));

        // 弹窗提示
        if (found) {
            QMessageBox::information(
                this,
                QStringLiteral("二叉搜索树查找"),
                QStringLiteral("查找成功：已找到元素 %1").arg(value)
            );
        } else {
            QMessageBox::information(
                this,
                QStringLiteral("二叉搜索树查找"),
                QStringLiteral("查找失败：未找到元素 %1").arg(value)
            );
        }
    });

    timer.start();
}

void MainWindow::bstInsert() {
    bool ok = false;
    int value = bstValue->text().toInt(&ok);
    if (!ok) {
        showMessage(QStringLiteral("二叉搜索树：请输入有效的键值"));
        return;
    }

    // 若已存在则弹窗并退出（保持原有逻辑）
    if (bst.find(value) != nullptr) {
        showMessage(QStringLiteral("BST：键值 %1 已存在").arg(value));
        QMessageBox::information(
            this,
            QStringLiteral("二叉搜索树插入"),
            QStringLiteral("插入失败：元素 %1 已经存在于树中").arg(value)
        );
        return;
    }

    // 预先从当前树中计算“查找插入位置”的路径（只存 key，不存指针）
    QVector<int> pathKeys;
    ds::BTNode* p = bst.root();
    while (p) {
        pathKeys.push_back(p->key);
        p = (value < p->key) ? p->left : p->right;
    }

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 第 1 步：确保树处于“插入前”的状态，并显示起始画面
    steps.push_back([this, value]() {
        // 如果树中已经包含 value（说明是“重播”），先删掉它，回到插入前的状态
        if (bst.find(value) != nullptr) {
            bst.eraseKey(value);
        }

        view->resetScene();
        view->setTitle(QStringLiteral("BST 插入 %1：查找位置（开始）").arg(value));
        drawBT(bst.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("BST 插入 %1：从根结点开始查找插入位置").arg(value));
    });

    // 逐步高亮查找路径（使用 key，每一帧都从当前树重新 find）
    for (int i = 0; i < pathKeys.size(); ++i) {
        const int keyOnPath = pathKeys[i];
        steps.push_back([this, value, keyOnPath, i, pathKeys]() {
            // 重播途中如果 value 还在树里，先删掉，保证仍是“插入前”的树
            if (bst.find(value) != nullptr) {
                bst.eraseKey(value);
            }

            ds::BTNode* node = bst.find(keyOnPath);

            view->resetScene();
            view->setTitle(QStringLiteral("BST 插入 %1：查找位置（%2/%3）").arg(value).arg(i + 1).arg(pathKeys.size()));

            g_btHighlightNode = node;
            drawBT(bst.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("BST 插入：经过结点 %1").arg(keyOnPath));
        });
    }

    // 最后一步：真正插入并高亮新结点
    steps.push_back([this, value]() {
        // 确保当前没有 value 再插入（避免重播时二次插入）
        if (bst.find(value) != nullptr) {
            bst.eraseKey(value);
        }

        bst.insert(value);
        ds::BTNode* node = bst.find(value);

        view->resetScene();
        g_btHighlightNode = node;
        drawBT(bst.root(), 400, 120, 200, 0);
        g_btHighlightNode = nullptr;

        showMessage(QStringLiteral("BST 插入完成：%1").arg(value));
    });

    timer.start();
    updateAnimUiState();
}

void MainWindow::bstErase() {
    bool ok = false;
    int value = bstValue->text().toInt(&ok);
    if (!ok) {
        showMessage(QStringLiteral("二叉搜索树：请输入有效的键值"));
        return;
    }

    // 先判断当前树中是否存在该结点
    ds::BTNode* target = bst.find(value);
    if (!target) {
        // 不存在：只给一次静态提示，这种情况没有“删除前的树”，重播也只会重复这个提示
        timer.stop();
        steps.clear();
        stepIndex = 0;
        //显示初始画面
        steps.push_back([this, value]() {
            view->resetScene();
            view->setTitle(QStringLiteral("BST 删除 %1：结点不存在").arg(value));
            drawBT(bst.root(), 400, 120, 200, 0);
            showMessage(QStringLiteral("BST 删除失败：未找到结点 %1").arg(value));
            QMessageBox::information(
                this,
                QStringLiteral("二叉搜索树删除"),
                QStringLiteral("删除失败：未找到要删除的元素 %1").arg(value)
            );
        });

        timer.start();
        updateAnimUiState();
        return;
    }

    // ========= 1. 在“删除前”的树上计算搜索路径（只存路径上的 key）=========
    QVector<int> pathKeys;
    ds::BTNode* p = bst.root();
    while (p && p->key != value) {
        pathKeys.push_back(p->key);
        p = (value < p->key) ? p->left : p->right;
    }
    if (p) {
        // 把要删除的结点本身也放进去
        pathKeys.push_back(p->key);
    }

    // ========= 2. 记录“删除前”的整棵 BST 结构（先序遍历的 key 序列）=========
    QVector<int> preorderKeys;
    dumpPreorder(bst.root(), preorderKeys);   // 利用你已有的辅助函数

    // ========= 3. 构造动画步骤 =========
    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 步骤 0：每次播放（包括重播）都先把 BST 精确还原到“删除前”的状态
    steps.push_back([this, value, preorderKeys]() {
        bst.clear();
        for (int k : preorderKeys) {
            bst.insert(k);
        }

        view->resetScene();
        view->setTitle(QStringLiteral("BST 删除 %1：查找目标（开始）").arg(value));
        drawBT(bst.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("BST 删除 %1：从根结点开始查找目标结点").arg(value));
    });

    // 步骤 1~n：按照 pathKeys 依次高亮搜索路径上的结点
    for (int i = 0; i < pathKeys.size(); ++i) {
        int keyOnPath = pathKeys[i];
        steps.push_back([this, value, keyOnPath, i, pathKeys, preorderKeys]() {
            // 为了避免上一次播放遗留的结构，这里也强制还原一遍“删除前”的树
            bst.clear();
            for (int k : preorderKeys) {
                bst.insert(k);
            }

            // 模拟搜索过程：从根一路查找，直到 keyOnPath
            ds::BTNode* cur = bst.root();
            ds::BTNode* highNode = nullptr;
            while (cur) {
                if (cur->key == keyOnPath) {
                    highNode = cur;
                    break;
                }
                if (value < cur->key) cur = cur->left;
                else cur = cur->right;
            }

            view->resetScene();
            view->setTitle(QStringLiteral("BST 删除 %1：路径（%2/%3）").arg(value).arg(i + 1).arg(pathKeys.size()));

            g_btHighlightNode = highNode;
            drawBT(bst.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("BST 删除：访问结点 %1").arg(keyOnPath));
        });
    }

    // 最后一步：在“删除前”的树上真正执行删除，并重绘结果
    steps.push_back([this, value, preorderKeys]() {
        // 先还原“删除前”的整棵树
        bst.clear();
        for (int k : preorderKeys) {
            bst.insert(k);
        }

        // 再真正执行一次删除
        bst.eraseKey(value);

        view->resetScene();
        view->setTitle(QStringLiteral("BST 删除 %1：删除完成").arg(value));
        drawBT(bst.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("BST 删除完成：%1").arg(value));
    });

    timer.start();
    updateAnimUiState();
}



void MainWindow::bstClear() { bst.clear(); view->resetScene(); view->setTitle(QStringLiteral("BST（空）")); }

// ===== 哈夫曼树（含合并动画） =====
void MainWindow::huffmanBuild() {
    auto w = parseIntList(huffmanInput->text());
    if (w.isEmpty()) {
        view->resetScene();
        view->setTitle(QStringLiteral("哈夫曼树：请输入权值序列"));
        showMessage(QStringLiteral("哈夫曼树：无有效输入"));
        return;
    }

    // 记录最后一次权值序列，便于保存/恢复
    huffLastWeights_ = w;

    huff.clear();
    timer.stop();
    steps.clear();
    stepIndex = 0;

    //给边添加 “0/1” 标签
    auto addEdgeLabel = [this](const QPointF& a, const QPointF& b, const QString& text, qreal offset = 12.0) {
        QPointF mid((a.x() + b.x()) / 2.0, (a.y() + b.y()) / 2.0);
        qreal vx = b.x() - a.x(), vy = b.y() - a.y();
        qreal L  = std::sqrt(vx * vx + vy * vy);
        qreal nx = 0.0, ny = -1.0;
        if (L > 1e-6) {
            nx = -vy / L;
            ny =  vx / L;
        }
        QPointF pos = mid + QPointF(nx * offset, ny * offset);//文字放置点
        auto* t = view->Scene()->addText(text);
        t->setDefaultTextColor(QColor("#111"));
        QRectF tb = t->boundingRect();
        t->setPos(pos.x() - tb.width() / 2.0, pos.y() - tb.height() / 2.0);
    };

    const qreal R = 34;
    using DrawFn = std::function<void(ds::BTNode*, qreal, qreal, qreal, const QString&, bool)>;
    auto drawHuffTree = std::make_shared<DrawFn>();
    //递归画图函数
    *drawHuffTree = [this, drawHuffTree, addEdgeLabel, R](ds::BTNode* n, qreal x, qreal y, qreal dist, const QString& prefix, bool annotateCodes) {
        if (!n) return;
        bool isLeaf = (!n->left && !n->right);//叶子

        // 叶结点高亮，内部结点普通（颜色由 Canvas::addNode 控制）
        view->addNode(x, y, QString::number(n->key), isLeaf);

        if (n->left) {
            qreal lx = x - dist, ly = y + 100;
            QPointF a(x, y + R), b(lx, ly - R);
            view->addEdge(a, b);
            addEdgeLabel(a, b, "0", 12.0);
            (*drawHuffTree)(n->left, lx, ly, dist / 1.8, prefix + "0", annotateCodes);
        }
        if (n->right) {
            qreal rx = x + dist, ry = y + 100;
            QPointF a(x, y + R), b(rx, ry - R);
            view->addEdge(a, b);
            addEdgeLabel(a, b, "1", 12.0);
            (*drawHuffTree)(n->right, rx, ry, dist / 1.8, prefix + "1", annotateCodes);
        }

        // 叶子上方标出码字
        if (annotateCodes && isLeaf) {
            QString code = prefix.isEmpty() ? QString("0") : prefix;
            auto* t = view->Scene()->addText(code);
            t->setDefaultTextColor(QColor("#065f46"));
            QRectF tb = t->boundingRect();
            t->setPos(x - tb.width() / 2.0, y - R - 12 - tb.height());
        }
    };

    // 小工具：从已构建好的 Huffman 树中收集所有叶子的 (权值, 编码)
    using CodePair = QPair<int, QString>;
    auto collectCodes = [](ds::BTNode* n, const QString& prefix, QVector<CodePair>& out, auto&& self) -> void {
        if (!n) return;
        bool isLeaf = (!n->left && !n->right);//叶子
        if (isLeaf) {
            QString code = prefix.isEmpty() ? QString("0") : prefix;
            out.push_back(qMakePair(n->key, code));
            return;
        }
        self(n->left, prefix + "0", out, self);
        self(n->right, prefix + "1", out, self);
    };

    QVector<ds::BTNode*> forest;
    forest.reserve(w.size());
    for (int x : w) forest.push_back(ds::Huffman::makeNode(x));//对每个权值创建一个独立节点（单节点树），作为 Huffman 合并的起点

    //森林静态布局
    auto drawForestFixed = [=, this](const QVector<ds::BTNode*>& F, const QString& title) {
        view->resetScene();
        view->setTitle(title);
        qreal x = 150;
        for (int i = 0; i < F.size(); ++i) {
            (*drawHuffTree)(F[i], x, 120, 60, "", false);
            x += 180;
        }
    };

    //两棵最小树“向中间移动”的补间动画
    auto tweenTwo = [=, this](const QVector<ds::BTNode*>& F, int i1, int i2, qreal t, const QString& title) {
        view->resetScene();
        view->setTitle(title);
        QVector<qreal> xs(F.size());
        qreal x0 = 150;
        for (int i = 0; i < F.size(); ++i) {
            xs[i] = x0 + i * 180;
        }
        qreal mid = (xs[i1] + xs[i2]) / 2.0;
        qreal xi1 = lerp(xs[i1], mid - 40, t);
        qreal xi2 = lerp(xs[i2], mid + 40, t);
        for (int i = 0; i < F.size(); ++i) {
            qreal x = xs[i];
            if (i == i1) x = xi1;
            if (i == i2) x = xi2;
            (*drawHuffTree)(F[i], x, 120, 60, "", false);
        }
    };

    QVector<ds::BTNode*> cur = forest;
    steps.push_back([=, this]() {
        drawForestFixed(cur, QStringLiteral("哈夫曼树：初始森林（%1 棵）").arg(cur.size()));
        statusBar()->showMessage(QStringLiteral("哈夫曼树：开始构建"));
    });

    const int tweenFrames = 8;
    const int tweenInterval = qMax(15, timer.interval() / 7);

    // 逐步合并两棵最小树，加入动画
    while (cur.size() > 1) {//只要森林里还多于 1 棵树，就继续合并
        int i1 = -1, i2 = -1;
        //选择最小两棵
        for (int i = 0; i < cur.size(); ++i)
            if (i1 == -1 || cur[i]->key < cur[i1]->key) i1 = i;
        for (int i = 0; i < cur.size(); ++i)
            if (i != i1 && (i2 == -1 || cur[i]->key < cur[i2]->key)) i2 = i;
        if (i1 > i2) std::swap(i1, i2);

        //生成父节点，其左右孩子指向两棵最小树
        int a = cur[i1]->key, b = cur[i2]->key;
        ds::BTNode* parent = ds::Huffman::makeNode(a + b);
        parent->left = cur[i1];
        parent->right = cur[i2];

        //合并前的快照
        QVector<ds::BTNode*> before = cur;
        QVector<ds::BTNode*> after = cur;
        after[i1] = parent;
        after.remove(i2);

        //插入一步：把合并前森林画出来，并在标题里强调当前选中哪两个最小权值
        steps.push_back([=, this]() {
            drawForestFixed(before, QStringLiteral("哈夫曼树：选择最小两棵：%1 与 %2").arg(a).arg(b));
        });

        //两棵最小的树靠近的动画
        for (int f = 0; f <= tweenFrames; ++f) {
            qreal t = qreal(f) / tweenFrames;
            steps.push_back([=, this]() {
                timer.setInterval(tweenInterval);
                tweenTwo(before, i1, i2, t, QStringLiteral("哈夫曼树：合并中（移动）"));
            });
        }

        //展示这一步合并之后的完整树
        steps.push_back([=, this]() {
            drawForestFixed(after, QStringLiteral("哈夫曼树：合并 %1 + %2 -> %3").arg(a).arg(b).arg(parent->key));
            onAnimSpeedChanged(animSpeedSlider->value());
        });

        //真正更新当前森林：用父节点替换 i1，并删除 i2
        cur[i1] = parent;
        cur.remove(i2);
    }

    if (!cur.isEmpty()) huff.rootNode = cur[0];

    // 最终：整棵树 + 叶子码字 + 右侧编码表
    steps.push_back([=, this]() {
        view->resetScene();
        view->setTitle(QStringLiteral("哈夫曼树：构建完成（边标 0/1；叶子上方显示码字）"));
        (*drawHuffTree)(huff.root(), 400, 120, 200, "", true);

        auto* legend = view->Scene()->addText(
            QStringLiteral("图例：黄色=原始叶结点   蓝绿色=内部结点（合并产生）"));
        legend->setDefaultTextColor(QColor("#444"));
        legend->setPos(16, 54);

        // ===== 同步更新右侧编码表 =====
        if (huffmanCodeTable) {
            huffmanCodeTable->setRowCount(0);

            QVector<CodePair> codes;
            collectCodes(huff.root(), "", codes, collectCodes);

            // 为了展示更整齐：按权值从小到大排序，同权值按码长排序
            std::sort(codes.begin(), codes.end(), [](const CodePair& a, const CodePair& b) {
                if (a.first != b.first) return a.first < b.first;
                return a.second.length() < b.second.length();
            });

            huffmanCodeTable->setRowCount(codes.size());
            for (int i = 0; i < codes.size(); ++i) {
                const int wVal = codes[i].first;
                const QString code = codes[i].second;

                auto* itemIdx = new QTableWidgetItem(QString::number(i + 1));
                itemIdx->setTextAlignment(Qt::AlignCenter);

                auto* itemW = new QTableWidgetItem(QString::number(wVal));
                itemW->setTextAlignment(Qt::AlignCenter);

                auto* itemCode = new QTableWidgetItem(code);
                itemCode->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

                huffmanCodeTable->setItem(i, 0, itemIdx);
                huffmanCodeTable->setItem(i, 1, itemW);
                huffmanCodeTable->setItem(i, 2, itemCode);
            }
        }

        showMessage(QStringLiteral("哈夫曼树：完成"));
    });

    timer.start();
}



void MainWindow::huffmanClear() {
    huff.clear();
    huffLastWeights_.clear();

    // 清空右侧编码表
    if (huffmanCodeTable) {
        huffmanCodeTable->setRowCount(0);
    }

    view->resetScene();
    view->setTitle(QStringLiteral("哈夫曼树（空）"));
    statusBar()->showMessage(QStringLiteral("哈夫曼树：已清空"));
}

// ===== AVL树 =====
void MainWindow::avlBuild() {
    auto a = parseIntList(avlInput->text());

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 第 0 步：清空 + 开始构建
    steps.push_back([this]() {
        avl.clear();
        view->resetScene();
        view->setTitle(QStringLiteral("AVL树：开始构建"));
        drawBT(avl.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("AVL树：开始构建"));
    });

    const int total = a.size();
    QVector<int> built; built.reserve(total);

    for (int i = 0; i < total; ++i) {
        const int v = a[i];
        // 这里“等价于调用 avlInsert 的核心动画逻辑”，但不会清空 steps
        drawAVL(v, built, i, total);
        built.push_back(v);
    }

    timer.start();
    updateAnimUiState();
}


void MainWindow::avlInsert() {
    bool ok = false;
    int value = avlValue->text().toInt(&ok);
    if (!ok) {
        showMessage(QStringLiteral("AVL树：请输入有效的键值"));
        return;
    }

    // 新增：记录“插入前”的整棵树（先序遍历的键序列）
    QVector<int> preorderKeys;
    dumpPreorder(avl.root(), preorderKeys);

    timer.stop();
    steps.clear();
    stepIndex = 0;

    drawAVL(value, preorderKeys, -1, -1);

    timer.start();
    updateAnimUiState();
}


void MainWindow::avlClear() {
    avl.clear();

    timer.stop();
    steps.clear();
    stepIndex = 0;

    view->resetScene();
    view->setTitle(QStringLiteral("AVL树（空）"));
    showMessage(QStringLiteral("AVL树：已清空"));
}

// ===== 绘制基础 =====
void MainWindow::drawSeqlist(const ds::Seqlist& sl){
    view->setCurrentFamily(QStringLiteral("seq"));
    view->resetScene();
    view->setTitle(QStringLiteral("顺序表"));

    const int n = sl.size();
    const qreal cellW = 68, cellH = 54, gap = 14;
    const qreal startX = 80, startY = 180;

    for (int i = 0; i < n; ++i) {
        const qreal x = startX + i * (cellW + gap);
        const qreal y = startY;

        view->addBox(x, y, cellW, cellH, QString::number(sl.get(i)), false);

        auto* idx = view->Scene()->addText(QString::number(i));
        idx->setDefaultTextColor(Qt::darkGray);
        idx->setPos(x + cellW / 2 - 6, y + cellH + 6);
    }
}

void MainWindow::drawLinklist(const ds::Linklist& ll){
    view->setCurrentFamily(QStringLiteral("link"));
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
    view->setCurrentFamily(QStringLiteral("stack"));
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
        view->addBox(leftX, yTop, innerW, BLOCK_H, QString::number(st.get(i)), isTop);

    }
    const qreal yTopBlock = bottomInnerY - n * BLOCK_H - (n - 1) * GAP; const QPointF target(leftX + innerW/2, yTopBlock);
    auto* t = S->addText("TOP"); t->setDefaultTextColor(Qt::red); QRectF tb = t->boundingRect(); const QPointF tagPos(x0 + W + 16, yTopBlock - tb.height()/2); t->setPos(tagPos);
    QPointF a(tagPos.x() + tb.width()/2, tagPos.y() + tb.height()/2); view->addEdge(a, target);
    S->setSceneRect(S->itemsBoundingRect().adjusted(-40, -40, 160, 80));
}

void MainWindow::drawBT(ds::BTNode* root, qreal x, qreal y, qreal /*distance*/, int /*highlightKey*/)
{
    if (!root) return;

    // 1) 中序遍历：为每个结点分配一个唯一的横向序号 idx（0,1,2,...）
    QHash<ds::BTNode*, int> xIndex;     // 结点 -> 横向序号
    QHash<ds::BTNode*, int> depth;      // 结点 -> 层深
    int idx = 0;

    std::function<void(ds::BTNode*, int)> inorder = [&](ds::BTNode* p, int d){
        if (!p) return;
        inorder(p->left,  d + 1);
        xIndex[p] = idx++;
        depth[p] = d;
        inorder(p->right, d + 1);
    };
    inorder(root, 0);
    if (idx <= 0) return;

    // 2) 把“序号坐标”映射为像素坐标，并整体居中
    const qreal stepX = 80.0;   // 横向固定步长
    const qreal levelH = 100.0;  // 纵向层间距
    const qreal mid = ( (idx - 1) * stepX ) / 2.0; // 全体宽度的中心，用来居中到 x

    QHash<ds::BTNode*, QPointF> pos;    // 结点 -> 像素坐标
    for (auto it = xIndex.constBegin(); it != xIndex.constEnd(); ++it) {
        ds::BTNode* n = it.key();
        qreal px = x + it.value() * stepX - mid;   // 居中平移
        qreal py = y + depth[n] * levelH;
        pos[n] = QPointF(px, py);
    }

    // 3) 先画边再画点（确保连线在底层，圆点在上层）
    std::function<void(ds::BTNode*)> drawRec = [&](ds::BTNode* p){
        if (!p) return;
        const QPointF cp = pos[p];

        if (p->left) {
            const QPointF cl = pos[p->left];
            view->addEdge(QPointF(cp.x(), cp.y() + 34), QPointF(cl.x(), cl.y() - 34));
            drawRec(p->left);
        }
        if (p->right) {
            const QPointF cr = pos[p->right];
            view->addEdge(QPointF(cp.x(), cp.y() + 34), QPointF(cr.x(), cr.y() - 34));
            drawRec(p->right);
        }

        // 关键改动：只按“结点指针”高亮，完全不再看 key
        const bool hl = (p == g_btHighlightNode);
        view->addNode(cp.x(), cp.y(), QString::number(p->key), hl);
    };
    drawRec(root);
}

void MainWindow::drawAVL(int v, const QVector<int>& restoreKeys, int idx, int total) {
    // 步骤1：插入前的静态画面
        steps.push_back([=, this]() {
            avl.clear();
            for (int k : restoreKeys) avl.insert(k);
            view->resetScene();
            view->setTitle(
                QStringLiteral("AVL树：准备插入 %1（第 %2/%3 步）").arg(v).arg(idx + 1).arg(total)
            );
            drawBT(avl.root(), 400, 120, 200, 0);
            showMessage(QStringLiteral("AVL树：准备插入 %1").arg(v));
        });

        // 步骤2：执行插入 + 若有旋转则播放“丝滑旋转动画”
        steps.push_back([=, this]() {
            // 小工具：根据当前树结构计算结点坐标（和 drawBT 一样的布局）
            auto computePos = [](ds::BTNode* root, qreal baseX, qreal baseY, QHash<ds::BTNode*, QPointF>& pos) {
                pos.clear();
                if (!root) return;

                QHash<ds::BTNode*, int> xIndex;
                QHash<ds::BTNode*, int> depth;
                int idx = 0;

                std::function<void(ds::BTNode*, int)> inorder =
                    [&](ds::BTNode* p, int d) {
                        if (!p) return;
                        inorder(p->left,  d + 1);
                        xIndex[p] = idx++;
                        depth[p]  = d;
                        inorder(p->right, d + 1);
                    };
                inorder(root, 0);
                if (idx <= 0) return;

                const qreal stepX  = 80.0;
                const qreal levelH = 100.0;
                const qreal mid    = ((idx - 1) * stepX) / 2.0;

                for (auto it = xIndex.constBegin(); it != xIndex.constEnd(); ++it) {
                    ds::BTNode* n = it.key();
                    qreal px = baseX + it.value() * stepX - mid;
                    qreal py = baseY + depth.value(n) * levelH;
                    pos[n] = QPointF(px, py);
                }
            };

            // 1）插入前布局
            QHash<ds::BTNode*, QPointF> posBefore;
            computePos(avl.root(), 400, 120, posBefore);

            // 2）真正插入（这里会做 LL/RR/LR/RL 旋转，并记录 rotationRecords）
            avl.insert(v);
            const auto& recs = avl.rotationRecords();

            // 3）插入后布局
            QHash<ds::BTNode*, QPointF> posAfter;
            computePos(avl.root(), 400, 120, posAfter);

            // 4）没有失衡，直接静态显示
            if (recs.empty()) {
                view->resetScene();
                view->setTitle(
                    QStringLiteral("AVL树：插入 %1（无需旋转）").arg(v)
                );
                drawBT(avl.root(), 400, 120, 200, 0);
                showMessage(
                    QStringLiteral("AVL树：插入 %1 后仍然平衡，无需旋转").arg(v)
                );
                return;
            }

            // 5）有失衡：播放从 posBefore -> posAfter 的平滑动画
            const auto& r = recs.front();
            QString typeStr;
            switch (r.type) {
                case ds::AVL::RotationRecord::LL: typeStr = QStringLiteral("LL"); break;
                case ds::AVL::RotationRecord::RR: typeStr = QStringLiteral("RR"); break;
                case ds::AVL::RotationRecord::LR: typeStr = QStringLiteral("LR"); break;
                case ds::AVL::RotationRecord::RL: typeStr = QStringLiteral("RL"); break;
            }

            // 暂停外层 steps 的定时器，交给 QTimeLine 播放细粒度动画
            timer.stop();
            // GIF 导出时：标记正在进行细粒度动画（避免 playSteps 误判“已结束”提前收尾）
            ++gifActiveTimelines_;

            const int frames = 18;   // 帧数越多越丝滑
            const int duration = 700;  // 动画总时长（毫秒）

            QTimeLine* tl = new QTimeLine(duration, this);
            tl->setFrameRange(0, frames);

            // 每一帧：重画整棵树，结点按 posBefore -> posAfter 插值移动
            connect(tl, &QTimeLine::frameChanged, this, [=, this](int frame) {
                qreal t = (frames == 0) ? 1.0 : qreal(frame) / frames;

                view->resetScene();
                view->setTitle(QStringLiteral("AVL树：插入 %1（%2 旋转动画 %3/%4）").arg(v).arg(typeStr).arg(frame).arg(frames));

                // 当前帧的坐标
                QHash<ds::BTNode*, QPointF> posNow;// 用来存当前帧每个节点的坐标
                for (auto it = posAfter.constBegin(); it != posAfter.constEnd(); ++it) {
                    ds::BTNode* node = it.key();
                    QPointF pAfter = it.value();
                    QPointF pBefore = posBefore.contains(node) ? posBefore.value(node) : pAfter; // 新结点：从最终位置“长出来”
                    qreal xNow = lerp(pBefore.x(), pAfter.x(), t);//当前位置 = 起点 + (终点 - 起点) * t
                    qreal yNow = lerp(pBefore.y(), pAfter.y(), t);
                    posNow.insert(node, QPointF(xNow, yNow));
                }

                // 递归函数，用来画整棵树
                // 画边 + 点（结构用当前真正的 AVL 树，坐标用插值后的）
                std::function<void(ds::BTNode*)> drawRec =
                    [&](ds::BTNode* p) {
                        if (!p) return;
                        QPointF cp = posNow.value(p);

                        // 递归画左子树，并连线
                        if (p->left) {
                            QPointF cl = posNow.value(p->left);
                            view->addEdge(QPointF(cp.x(), cp.y() + 34),
                                          QPointF(cl.x(), cl.y() - 34));
                            drawRec(p->left);
                        }
                        // 递归画右子树，并连线
                        if (p->right) {
                            QPointF cr = posNow.value(p->right);
                            view->addEdge(QPointF(cp.x(), cp.y() + 34),
                                          QPointF(cr.x(), cr.y() - 34));
                            drawRec(p->right);
                        }
                        // 画节点圆圈
                        bool hl = (p == r.z || p == r.y || p == r.x);
                        view->addNode(cp.x(), cp.y(), QString::number(p->key), hl);
                    };
                // 真正开始画树（从根节点开始）
                drawRec(avl.root());

                if (frame == frames) {
                    QString msg = QStringLiteral("AVL树：插入 %1 后，结点 %2 失衡，进行了 %3 旋转").arg(v).arg(r.z ? QString::number(r.z->key) : QStringLiteral("?")).arg(typeStr);
                    if (r.y) msg += QStringLiteral("，y = %1").arg(r.y->key);
                    if (r.x) msg += QStringLiteral("，x = %1").arg(r.x->key);
                    showMessage(msg);
                }
            });

            // 动画结束：恢复外层 timer（如果还有后续步骤）
            connect(tl, &QTimeLine::finished, this, [=, this]() {
                tl->deleteLater();
                // GIF 导出时：细粒度动画结束
                if (gifActiveTimelines_ > 0) --gifActiveTimelines_;
                // 如果正在导出 GIF，且此时已无后续 steps，则尝试收尾写文件
                maybeFinishGifExport();

                if (stepIndex < steps.size()) {
                    timer.start();          // 继续播放后面的步骤
                } else {
                    showMessage(QStringLiteral("播放结束"));
                }
            });

            tl->start();
        });
}