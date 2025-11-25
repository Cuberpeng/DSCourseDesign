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
#include <QHash>
#include <QTableWidget>
#include <QHeaderView>
#include <QPointF>

static ds::BTNode* g_btHighlightNode = nullptr;
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

    const qreal cellW  = 68;
    const qreal cellH  = 54;
    const qreal gap    = 14;
    const qreal startX = 80;
    const qreal startY = 180;

    const int oldInterval = timer.interval();
    const int animInterval = 60;

    // 根据尾部长度控制每个“小动画”的帧数，避免元素过多时太慢
    int tail = n - pos;
    int framesShift = 10;
    if (tail > 80)  framesShift = 6;
    if (tail > 200) framesShift = 4;

    const int framesDrop = 10;

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 步骤 0：显示当前状态，高亮插入位置
    steps.push_back([=]() {
        view->resetScene();
        view->setTitle(QStringLiteral("顺序表：插入前（pos=%1）").arg(pos));

        for (int i = 0; i < n; ++i) {
            qreal x = startX + i * (cellW + gap);
            qreal y = startY;
            view->Scene()->addRect(QRectF(x, y, cellW, cellH),
                                   QPen(QColor("#5f6c7b"), 2),
                                   QBrush(QColor("#e8eef9")));
            auto *t = view->Scene()->addText(arr[i]);
            t->setDefaultTextColor(Qt::black);
            QRectF tb = t->boundingRect();
            t->setPos(x + (cellW - tb.width()) / 2,
                      y + (cellH - tb.height()) / 2 - 1);

            auto *idx = view->Scene()->addText(QString::number(i));
            idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x + cellW / 2 - 6, y + cellH + 6);
        }

        // 高亮插入位置
        qreal hx = startX + pos * (cellW + gap);
        view->Scene()->addRect(QRectF(hx, startY, cellW, cellH),
                               QPen(QColor("#ef4444"), 3),
                               QBrush(Qt::transparent));
        showMessage(QStringLiteral("顺序表：准备在位置 %1 插入").arg(pos));
    });

    // 步骤 1：把计时器调成动画间隔
    steps.push_back([=]() {
        timer.setInterval(animInterval);
    });

    // 步骤 2：从右往左，一个一个挪动元素 i -> i+1
    // k 表示当前在移动的元素下标
    for (int k = n - 1; k >= pos; --k) {
        for (int f = 0; f <= framesShift; ++f) {
            const qreal t = (framesShift == 0) ? 1.0 : (qreal)f / framesShift;

            steps.push_back([=]() {
                view->resetScene();
                view->setTitle(QStringLiteral("顺序表：移动元素 %1 → %2")
                               .arg(k).arg(k + 1));

                for (int j = 0; j < n; ++j) {
                    // 计算“当前这一帧里，这个元素显示在第几个格子上”
                    qreal idxPos;  // 可能是小数，用来做插值

                    if (j < pos) {
                        // 插入位置之前的元素不动
                        idxPos = j;
                    } else if (j > k) {
                        // 已经完成移动的右边部分：全部在最终位置 j+1
                        idxPos = j + 1;
                    } else if (j == k) {
                        // 当前要移动的这个元素：从 k 挪到 k+1
                        idxPos = (qreal)k + t;
                    } else {
                        // pos <= j < k：还没轮到移动，保持在位置 j
                        idxPos = j;
                    }

                    qreal x = startX + idxPos * (cellW + gap);
                    qreal y = startY;

                    bool highlight = (j == k);
                    QBrush boxBrush(highlight ? QColor("#ffd166") : QColor("#e8eef9"));
                    view->Scene()->addRect(QRectF(x, y, cellW, cellH),
                                           QPen(QColor("#5f6c7b"), 2),
                                           boxBrush);

                    auto *tItem = view->Scene()->addText(arr[j]);
                    tItem->setDefaultTextColor(Qt::black);
                    QRectF tb = tItem->boundingRect();
                    tItem->setPos(x + (cellW - tb.width()) / 2,
                                  y + (cellH - tb.height()) / 2 - 1);
                }

                // 在整个移动过程中，新元素先停在插入位置上方，不参与移动
                qreal newX = startX + pos * (cellW + gap);
                qreal newY = startY - 80;
                view->Scene()->addRect(QRectF(newX, newY, cellW, cellH),
                                       QPen(QColor("#22c55e"), 2),
                                       QBrush(QColor("#bbf7d0")));
                auto *newText = view->Scene()->addText(QString::number(val));
                newText->setDefaultTextColor(Qt::black);
                QRectF tbNew = newText->boundingRect();
                newText->setPos(newX + (cellW - tbNew.width()) / 2,
                                newY + (cellH - tbNew.height()) / 2 - 1);
            });
        }
    }

    // 步骤 3：让新元素从上往下“掉”到 pos 位置
    for (int f = 0; f <= framesDrop; ++f) {
        const qreal t = (framesDrop == 0) ? 1.0 : (qreal)f / framesDrop;

        steps.push_back([=]() {
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
                    // 这里暂时留空位，让上面掉下来的块来填
                    boxBrush = QBrush(Qt::NoBrush);
                } else {
                    text = arr[i - 1];
                }

                view->Scene()->addRect(QRectF(x, y, cellW, cellH),
                                       QPen(QColor("#5f6c7b"), 2),
                                       boxBrush);
                if (!text.isEmpty()) {
                    auto *tItem = view->Scene()->addText(text);
                    tItem->setDefaultTextColor(Qt::black);
                    QRectF tb = tItem->boundingRect();
                    tItem->setPos(x + (cellW - tb.width()) / 2,
                                  y + (cellH - tb.height()) / 2 - 1);
                }
            }

            // 画“正在下落”的新元素
            qreal baseX = startX + pos * (cellW + gap);
            qreal startYTop = startY - 80;
            qreal curY = startYTop + (startY - startYTop) * t;

            view->Scene()->addRect(QRectF(baseX, curY, cellW, cellH),
                                   QPen(QColor("#22c55e"), 2),
                                   QBrush(QColor("#bbf7d0")));
            auto *newText = view->Scene()->addText(QString::number(val));
            newText->setDefaultTextColor(Qt::black);
            QRectF tbNew = newText->boundingRect();
            newText->setPos(baseX + (cellW - tbNew.width()) / 2,
                            curY + (cellH - tbNew.height()) / 2 - 1);
        });
    }

    // 步骤 4：真正往后端顺序表里插入 + 恢复计时器 + 画最终结果
    steps.push_back([=]() {
        timer.setInterval(oldInterval);

        seq.insert(pos, val);
        view->resetScene();
        view->setTitle(QStringLiteral("顺序表：插入完成（pos=%1, val=%2）")
                       .arg(pos).arg(val));
        drawSeqlist(seq);
        showMessage(QStringLiteral("顺序表：插入完成"));
    });

    timer.start();
}

void MainWindow::seqlistErase(){
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

    const int oldInterval = timer.interval();
    const int animInterval = 60;

    int tail = n - 1 - pos;
    int framesShift = 10;
    if (tail > 80)  framesShift = 6;
    if (tail > 200) framesShift = 4;

    const int framesDelete = 10;

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 步骤 0：显示当前状态，高亮要删除的格子
    steps.push_back([=]() {
        view->resetScene();
        view->setTitle(QStringLiteral("顺序表：删除前（pos=%1）").arg(pos));

        for (int i = 0; i < n; ++i) {
            bool hi = (i == pos);
            qreal x = startX + i * (cellW + gap);
            qreal y = startY;
            view->Scene()->addRect(QRectF(x, y, cellW, cellH),
                                   QPen(QColor("#5f6c7b"), 2),
                                   QBrush(hi ? QColor("#fecaca") : QColor("#e8eef9")));
            auto *t = view->Scene()->addText(arr[i]);
            t->setDefaultTextColor(Qt::black);
            QRectF tb = t->boundingRect();
            t->setPos(x + (cellW - tb.width()) / 2,
                      y + (cellH - tb.height()) / 2 - 1);

            auto *idx = view->Scene()->addText(QString::number(i));
            idx->setDefaultTextColor(Qt::darkGray);
            idx->setPos(x + cellW / 2 - 6, y + cellH + 6);
        }

        showMessage(QStringLiteral("顺序表：准备删除位置 %1 的元素").arg(pos));
    });

    // 步骤 1：把计时器调成动画间隔
    steps.push_back([=]() {
        timer.setInterval(animInterval);
    });

    // 步骤 2：把要删除的那个格子“抬上去 + 变透明”
    for (int f = 0; f <= framesDelete; ++f) {
        const qreal t = (framesDelete == 0) ? 1.0 : (qreal)f / framesDelete;

        steps.push_back([=]() {
            view->resetScene();
            view->setTitle(QStringLiteral("顺序表：删除元素"));

            // 其它格子原地不动（不画 pos）
            for (int i = 0; i < n; ++i) {
                if (i == pos) continue;
                qreal x = startX + i * (cellW + gap);
                qreal y = startY;
                view->Scene()->addRect(QRectF(x, y, cellW, cellH),
                                       QPen(QColor("#5f6c7b"), 2),
                                       QBrush(QColor("#e8eef9")));
                auto *tItem = view->Scene()->addText(arr[i]);
                tItem->setDefaultTextColor(Qt::black);
                QRectF tb = tItem->boundingRect();
                tItem->setPos(x + (cellW - tb.width()) / 2,
                              y + (cellH - tb.height()) / 2 - 1);
            }

            // 被删的元素向上抬起并逐渐透明
            qreal baseX = startX + pos * (cellW + gap);
            qreal baseY = startY;
            qreal curY  = baseY - 80 * t;
            qreal alpha = 1.0 - t;

            auto *rect = view->Scene()->addRect(QRectF(baseX, curY, cellW, cellH),
                                                QPen(QColor("#ef4444"), 2),
                                                QBrush(QColor("#fecaca")));
            rect->setOpacity(alpha);

            auto *text = view->Scene()->addText(arr[pos]);
            text->setDefaultTextColor(Qt::black);
            text->setOpacity(alpha);
            QRectF tb = text->boundingRect();
            text->setPos(baseX + (cellW - tb.width()) / 2,
                         curY + (cellH - tb.height()) / 2 - 1);
        });
    }

    // 步骤 3：从左到右，一个一个把后面的元素往前挪：k: pos+1 -> n-1
    for (int k = pos + 1; k < n; ++k) {
        for (int f = 0; f <= framesShift; ++f) {
            const qreal t = (framesShift == 0) ? 1.0 : (qreal)f / framesShift;

            steps.push_back([=]() {
                view->resetScene();
                view->setTitle(QStringLiteral("顺序表：移动元素 %1 → %2")
                               .arg(k).arg(k - 1));

                for (int j = 0; j < n; ++j) {
                    if (j == pos) continue; // 被删的元素不再画

                    // 计算这一帧中 arr[j] 所在的格子索引（可能是小数）
                    qreal idxPos;

                    if (j < pos) {
                        // 删除位置之前的元素不动
                        idxPos = j;
                    } else if (j > k) {
                        // 还没轮到移动的尾部元素：一直保持原位 j
                        idxPos = j;
                    } else if (j == k) {
                        // 当前正在移动的元素：从 k → k-1
                        idxPos = (qreal)k - t;
                    } else {
                        // pos < j < k：已经在之前的步骤中移动过一次，停在 j-1
                        idxPos = j - 1;
                    }

                    qreal x = startX + idxPos * (cellW + gap);
                    qreal y = startY;

                    bool highlight = (j == k);
                    QBrush boxBrush(highlight ? QColor("#ffd166") : QColor("#e8eef9"));
                    view->Scene()->addRect(QRectF(x, y, cellW, cellH),
                                           QPen(QColor("#5f6c7b"), 2),
                                           boxBrush);

                    auto *tItem = view->Scene()->addText(arr[j]);
                    tItem->setDefaultTextColor(Qt::black);
                    QRectF tb = tItem->boundingRect();
                    tItem->setPos(x + (cellW - tb.width()) / 2,
                                  y + (cellH - tb.height()) / 2 - 1);
                }
            });
        }
    }

    // 步骤 4：真正删除后端元素 + 恢复计时器 + 画最终顺序表
    steps.push_back([=]() {
        timer.setInterval(oldInterval);

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

    // 2) 前端再跑一遍先序，拿“结点指针序列”
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

            // ✅ 告诉 drawBT：这一次只高亮这个结点
            g_btHighlightNode = node;
            drawBT(bt.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("先序遍历：访问 %1").arg(key));
        });
    }

    timer.start();
}

void MainWindow::btInorder() {
    int need = bt.inorder(nullptr, 0);
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
    int need = bt.postorder(nullptr, 0);
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

    int need = bt.levelorder(nullptr, 0);
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
        steps.push_back([=]() { bst.insert(a[i]); view->resetScene(); view->setTitle(QStringLiteral("二叉搜索树：插入 %1（第 %2/%3 步）").arg(a[i]).arg(i+1).arg(a.size())); drawBT(bst.root(), 400, 120, 200, 0); });
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
            view->setTitle(QStringLiteral("BST 查找 %1（%2/%3）")
                .arg(value).arg(i+1).arg(path.size()));

            g_btHighlightNode = node;      // 按指针高亮
            drawBT(bst.root(), 400, 120, 200, 0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("BST 查找：比较 %1 和 %2")
                        .arg(node->key).arg(value));
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

        view->setTitle(QStringLiteral("BST 查找 %1：%2")
                       .arg(value).arg(found?QStringLiteral("找到"):QStringLiteral("未找到")));
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

    // 若已存在则弹窗并退出
    if (bst.find(value) != nullptr) {
        showMessage(QStringLiteral("BST：键值 %1 已存在").arg(value));
        QMessageBox::information(
            this,
            QStringLiteral("二叉搜索树插入"),
            QStringLiteral("插入失败：元素 %1 已经存在于树中").arg(value)
        );
        return;
    }

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 查找插入位置路径
    QVector<ds::BTNode*> path;
    ds::BTNode* p = bst.root();
    while (p) {
        path.push_back(p);
        p = (value < p->key) ? p->left : p->right;
    }

    // 空树情况
    if (path.isEmpty()) {
        steps.push_back([this,value]() {
            bst.insert(value);
            g_btHighlightNode = bst.root();
            view->resetScene();
            drawBT(bst.root(),400,120,200,0);
            g_btHighlightNode = nullptr;
            showMessage(QStringLiteral("BST 插入：空树插入 %1").arg(value));
        });
        timer.start();
        return;
    }

    // 高亮插入路径
    for (int i=0;i<path.size();++i) {
        ds::BTNode* node = path[i];
        steps.push_back([this,i,path,node,value]() {
            view->resetScene();
            view->setTitle(QStringLiteral("BST 插入 %1：查找位置（%2/%3）")
                           .arg(value).arg(i+1).arg(path.size()));

            g_btHighlightNode = node;   // 按指针高亮
            drawBT(bst.root(),400,120,200,0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("BST 插入：经过 %1").arg(node->key));
        });
    }

    // 插入并高亮新节点
    steps.push_back([this,value]() {
        bst.insert(value);
        ds::BTNode* node = bst.find(value);

        view->resetScene();
        g_btHighlightNode = node;
        drawBT(bst.root(),400,120,200,0);
        g_btHighlightNode = nullptr;

        showMessage(QStringLiteral("BST 插入完成：%1").arg(value));
    });

    timer.start();
}

void MainWindow::bstErase() {
    bool ok = false;
    int value = bstValue->text().toInt(&ok);
    if (!ok) {
        showMessage(QStringLiteral("二叉搜索树：请输入有效的键值"));
        return;
    }

    // 查找目标结点路径
    QVector<ds::BTNode*> path;
    ds::BTNode* p = bst.root();
    while (p && p->key != value) {
        path.push_back(p);
        p = (value < p->key) ? p->left : p->right;
    }
    if (p) path.push_back(p);

    bool found = (p != nullptr);

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 未找到要删除的元素：直接弹窗提示
    if (!found) {
        steps.push_back([this,value]() {
            view->resetScene();
            drawBT(bst.root(),400,120,200,0);
            showMessage(QStringLiteral("BST：未找到 %1").arg(value));
            QMessageBox::information(
                this,
                QStringLiteral("二叉搜索树删除"),
                QStringLiteral("删除失败：未找到要删除的元素 %1").arg(value)
            );
        });
        timer.start();
        return;
    }

    // 高亮查找路径
    for (int i=0;i<path.size();++i) {
        ds::BTNode* node = path[i];
        steps.push_back([this,i,path,node,value]() {
            view->resetScene();
            view->setTitle(QStringLiteral("BST 删除 %1：路径（%2/%3）")
                           .arg(value).arg(i+1).arg(path.size()));

            g_btHighlightNode = node;    // 按指针高亮
            drawBT(bst.root(),400,120,200,0);
            g_btHighlightNode = nullptr;

            showMessage(QStringLiteral("访问结点 %1").arg(node->key));
        });
    }

    // 删除并重新绘制（删除成功这里就不再额外弹窗了，按你原来的需求只对失败弹窗）
    steps.push_back([this,value]() {
        bst.eraseKey(value);
        view->resetScene();
        drawBT(bst.root(),400,120,200,0);
        showMessage(QStringLiteral("BST 删除完成：%1").arg(value));
    });

    timer.start();
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

    auto addEdgeLabel = [this](const QPointF& a, const QPointF& b, const QString& text, qreal offset = 12.0) {
        QPointF mid((a.x() + b.x()) / 2.0, (a.y() + b.y()) / 2.0);
        qreal vx = b.x() - a.x(), vy = b.y() - a.y();
        qreal L  = std::sqrt(vx * vx + vy * vy);
        qreal nx = 0.0, ny = -1.0;
        if (L > 1e-6) {
            nx = -vy / L;
            ny =  vx / L;
        }
        QPointF pos = mid + QPointF(nx * offset, ny * offset);
        auto* t = view->Scene()->addText(text);
        t->setDefaultTextColor(QColor("#111"));
        QRectF tb = t->boundingRect();
        t->setPos(pos.x() - tb.width() / 2.0, pos.y() - tb.height() / 2.0);
    };

    const qreal R = 34;
    using DrawFn = std::function<void(ds::BTNode*, qreal, qreal, qreal, const QString&, bool)>;
    auto drawHuffTree = std::make_shared<DrawFn>();
    *drawHuffTree = [this, drawHuffTree, addEdgeLabel, R](ds::BTNode* n,
                                                          qreal x, qreal y, qreal dist,
                                                          const QString& prefix,
                                                          bool annotateCodes) {
        if (!n) return;
        bool isLeaf = (!n->left && !n->right);

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
    auto collectCodes = [](ds::BTNode* n, const QString& prefix, QVector<CodePair>& out,
                           auto&& self) -> void {
        if (!n) return;
        bool isLeaf = (!n->left && !n->right);
        if (isLeaf) {
            QString code = prefix.isEmpty() ? QString("0") : prefix;
            out.push_back(qMakePair(n->key, code));
            return;
        }
        self(n->left,  prefix + "0", out, self);
        self(n->right, prefix + "1", out, self);
    };

    QVector<ds::BTNode*> forest;
    forest.reserve(w.size());
    for (int x : w) forest.push_back(ds::Huffman::makeNode(x));

    auto drawForestFixed = [=](const QVector<ds::BTNode*>& F, const QString& title) {
        view->resetScene();
        view->setTitle(title);
        qreal x = 150;
        for (int i = 0; i < F.size(); ++i) {
            (*drawHuffTree)(F[i], x, 120, 60, "", false);
            x += 180;
        }
    };

    auto tweenTwo = [=](const QVector<ds::BTNode*>& F, int i1, int i2, qreal t, const QString& title) {
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
    steps.push_back([=]() {
        drawForestFixed(cur, QStringLiteral("哈夫曼树：初始森林（%1 棵）").arg(cur.size()));
        statusBar()->showMessage(QStringLiteral("哈夫曼树：开始构建"));
    });

    const int tweenFrames = 8;
    const int oldInterval = timer.interval();

    // 逐步合并两棵最小树，加入动画
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
        after[i1] = parent;
        after.remove(i2);

        steps.push_back([=]() {
            drawForestFixed(before, QStringLiteral("哈夫曼树：选择最小两棵：%1 与 %2").arg(a).arg(b));
        });

        for (int f = 0; f <= tweenFrames; ++f) {
            qreal t = qreal(f) / tweenFrames;
            steps.push_back([=]() {
                timer.setInterval(60);
                tweenTwo(before, i1, i2, t, QStringLiteral("哈夫曼树：合并中（移动）"));
            });
        }

        steps.push_back([=]() {
            drawForestFixed(after, QStringLiteral("哈夫曼树：合并 %1 + %2 -> %3").arg(a).arg(b).arg(parent->key));
            timer.setInterval(oldInterval);
        });

        cur[i1] = parent;
        cur.remove(i2);
    }

    if (!cur.isEmpty()) huff.rootNode = cur[0];

    // 最终：整棵树 + 叶子码字 + 右侧编码表
    steps.push_back([=]() {
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

    for (int i = 0; i < total; ++i) {
        const int v   = a[i];
        const int idx = i;

        // 步骤1：插入前的静态画面
        steps.push_back([=]() {
            view->resetScene();
            view->setTitle(
                QStringLiteral("AVL树：准备插入 %1（第 %2/%3 步）")
                    .arg(v).arg(idx + 1).arg(total)
            );
            drawBT(avl.root(), 400, 120, 200, 0);
            showMessage(QStringLiteral("AVL树：准备插入 %1").arg(v));
        });

        // 步骤2：执行插入 + 若有旋转则播放“丝滑旋转动画”
        steps.push_back([=]() {
            // 小工具：根据当前树结构计算结点坐标（和 drawBT 一样的布局）
            auto computePos = [](ds::BTNode* root, qreal baseX, qreal baseY,
                                 QHash<ds::BTNode*, QPointF>& pos) {
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
                    pos[n]   = QPointF(px, py);
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

            const int frames   = 18;   // 帧数越多越丝滑
            const int duration = 700;  // 动画总时长（毫秒）

            QTimeLine* tl = new QTimeLine(duration, this);
            tl->setFrameRange(0, frames);

            // 每一帧：重画整棵树，结点按 posBefore -> posAfter 插值移动
            connect(tl, &QTimeLine::frameChanged, this,
                    [=](int frame) {
                qreal t = (frames == 0) ? 1.0 : qreal(frame) / frames;

                view->resetScene();
                view->setTitle(
                    QStringLiteral("AVL树：插入 %1（%2 旋转动画 %3/%4）")
                        .arg(v).arg(typeStr).arg(frame).arg(frames)
                );

                // 当前帧的坐标
                QHash<ds::BTNode*, QPointF> posNow;
                for (auto it = posAfter.constBegin(); it != posAfter.constEnd(); ++it) {
                    ds::BTNode* node   = it.key();
                    QPointF pAfter     = it.value();
                    QPointF pBefore    = posBefore.contains(node)
                                          ? posBefore.value(node)
                                          : pAfter; // 新结点：从最终位置“长出来”
                    qreal xNow = lerp(pBefore.x(), pAfter.x(), t);
                    qreal yNow = lerp(pBefore.y(), pAfter.y(), t);
                    posNow.insert(node, QPointF(xNow, yNow));
                }

                // 画边 + 点（结构用当前真正的 AVL 树，坐标用插值后的）
                std::function<void(ds::BTNode*)> drawRec =
                    [&](ds::BTNode* p) {
                        if (!p) return;
                        QPointF cp = posNow.value(p);

                        if (p->left) {
                            QPointF cl = posNow.value(p->left);
                            view->addEdge(QPointF(cp.x(), cp.y() + 34),
                                          QPointF(cl.x(), cl.y() - 34));
                            drawRec(p->left);
                        }
                        if (p->right) {
                            QPointF cr = posNow.value(p->right);
                            view->addEdge(QPointF(cp.x(), cp.y() + 34),
                                          QPointF(cr.x(), cr.y() - 34));
                            drawRec(p->right);
                        }

                        bool hl = (p == r.z || p == r.y || p == r.x);
                        view->addNode(cp.x(), cp.y(), QString::number(p->key), hl);
                    };
                drawRec(avl.root());

                if (frame == frames) {
                    QString msg =
                        QStringLiteral("AVL树：插入 %1 后，结点 %2 失衡，进行了 %3 旋转")
                            .arg(v)
                            .arg(r.z ? QString::number(r.z->key) : QStringLiteral("?"))
                            .arg(typeStr);
                    if (r.y) msg += QStringLiteral("，y = %1").arg(r.y->key);
                    if (r.x) msg += QStringLiteral("，x = %1").arg(r.x->key);
                    showMessage(msg);
                }
            });

            // 动画结束：恢复外层 timer（如果还有后续步骤）
            connect(tl, &QTimeLine::finished, this, [=]() {
                tl->deleteLater();
                if (stepIndex < steps.size()) {
                    timer.start();          // 继续播放后面的步骤
                } else {
                    showMessage(QStringLiteral("播放结束"));
                }
            });

            tl->start();
        });
    }

    timer.start();
}


void MainWindow::avlInsert() {
    bool ok = false;
    int value = avlValue->text().toInt(&ok);
    if (!ok) {
        showMessage(QStringLiteral("AVL树：请输入有效的键值"));
        return;
    }

    timer.stop();
    steps.clear();
    stepIndex = 0;

    // 步骤1：插入前静态画面
    steps.push_back([=]() {
        view->resetScene();
        view->setTitle(QStringLiteral("AVL树：插入前"));
        drawBT(avl.root(), 400, 120, 200, 0);
        showMessage(QStringLiteral("AVL树：准备插入 %1").arg(value));
    });

    // 步骤2：执行插入 + 播放旋转动画
    steps.push_back([=]() {
        auto computePos = [](ds::BTNode* root, qreal baseX, qreal baseY,
                             QHash<ds::BTNode*, QPointF>& pos) {
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
                pos[n]   = QPointF(px, py);
            }
        };

        // 插入前布局
        QHash<ds::BTNode*, QPointF> posBefore;
        computePos(avl.root(), 400, 120, posBefore);

        // 执行插入
        avl.insert(value);
        const auto& recs = avl.rotationRecords();

        // 插入后布局
        QHash<ds::BTNode*, QPointF> posAfter;
        computePos(avl.root(), 400, 120, posAfter);

        // 没有旋转：简单显示一下结果
        if (recs.empty()) {
            view->resetScene();
            view->setTitle(
                QStringLiteral("AVL树：插入 %1（无需旋转）").arg(value)
            );
            drawBT(avl.root(), 400, 120, 200, 0);
            showMessage(
                QStringLiteral("AVL树：插入 %1 后仍然平衡，无需旋转").arg(value)
            );
            return;
        }

        // 有旋转：播放平滑动画
        const auto& r = recs.front();
        QString typeStr;
        switch (r.type) {
        case ds::AVL::RotationRecord::LL: typeStr = QStringLiteral("LL"); break;
        case ds::AVL::RotationRecord::RR: typeStr = QStringLiteral("RR"); break;
        case ds::AVL::RotationRecord::LR: typeStr = QStringLiteral("LR"); break;
        case ds::AVL::RotationRecord::RL: typeStr = QStringLiteral("RL"); break;
        }

        timer.stop();

        const int frames   = 18;
        const int duration = 700;

        QTimeLine* tl = new QTimeLine(duration, this);
        tl->setFrameRange(0, frames);

        connect(tl, &QTimeLine::frameChanged, this,
                [=](int frame) {
            qreal t = (frames == 0) ? 1.0 : qreal(frame) / frames;

            view->resetScene();
            view->setTitle(
                QStringLiteral("AVL树：插入 %1（%2 旋转动画 %3/%4）")
                    .arg(value).arg(typeStr).arg(frame).arg(frames)
            );

            QHash<ds::BTNode*, QPointF> posNow;
            for (auto it = posAfter.constBegin(); it != posAfter.constEnd(); ++it) {
                ds::BTNode* node   = it.key();
                QPointF pAfter     = it.value();
                QPointF pBefore    = posBefore.contains(node)
                                      ? posBefore.value(node)
                                      : pAfter;
                qreal xNow = lerp(pBefore.x(), pAfter.x(), t);
                qreal yNow = lerp(pBefore.y(), pAfter.y(), t);
                posNow.insert(node, QPointF(xNow, yNow));
            }

            std::function<void(ds::BTNode*)> drawRec =
                [&](ds::BTNode* p) {
                    if (!p) return;
                    QPointF cp = posNow.value(p);

                    if (p->left) {
                        QPointF cl = posNow.value(p->left);
                        view->addEdge(QPointF(cp.x(), cp.y() + 34),
                                      QPointF(cl.x(), cl.y() - 34));
                        drawRec(p->left);
                    }
                    if (p->right) {
                        QPointF cr = posNow.value(p->right);
                        view->addEdge(QPointF(cp.x(), cp.y() + 34),
                                      QPointF(cr.x(), cr.y() - 34));
                        drawRec(p->right);
                    }

                    bool hl = (p == r.z || p == r.y || p == r.x);
                    view->addNode(cp.x(), cp.y(), QString::number(p->key), hl);
                };
            drawRec(avl.root());

            if (frame == frames) {
                QString msg =
                    QStringLiteral("AVL树：插入 %1 后，结点 %2 失衡，进行了 %3 旋转")
                        .arg(value)
                        .arg(r.z ? QString::number(r.z->key) : QStringLiteral("?"))
                        .arg(typeStr);
                if (r.y) msg += QStringLiteral("，y = %1").arg(r.y->key);
                if (r.x) msg += QStringLiteral("，x = %1").arg(r.x->key);
                showMessage(msg);
            }
        });

        connect(tl, &QTimeLine::finished, this, [=]() {
            tl->deleteLater();
            if (stepIndex < steps.size()) {
                timer.start();
            } else {
                showMessage(QStringLiteral("播放结束"));
            }
        });

        tl->start();
    });

    timer.start();
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
        depth[p]  = d;
        inorder(p->right, d + 1);
    };
    inorder(root, 0);
    if (idx <= 0) return;

    // 2) 把“序号坐标”映射为像素坐标，并整体居中
    const qreal stepX   = 80.0;   // 横向固定步长
    const qreal levelH  = 100.0;  // 纵向层间距
    const qreal mid     = ( (idx - 1) * stepX ) / 2.0; // 全体宽度的中心，用来居中到 x

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

        // ✅ 关键改动：只按“结点指针”高亮，完全不再看 key
        const bool hl = (p == g_btHighlightNode);
        view->addNode(cp.x(), cp.y(), QString::number(p->key), hl);
    };
    drawRec(root);
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
    QFileDialog dialog(this, QStringLiteral("保存为"), "", "XPR's DS Visualizer (*.xpr)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("xpr");
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
    QFileDialog dialog(this, QStringLiteral("打开"), "", "XPR's DS Visualizer (*.xpr)");
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
<p>规则：</p>
<ul>
  <li>每行一条命令；大小写不敏感；数字可用空格或逗号分隔。</li>
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


)HTML");

    doc->setHtml(html);

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
    if (low.isEmpty()) {
        showMessage(QStringLiteral("NLI：请输入自然语言指令"));
        return;
    }

    // —— 仅检测 NLI 内部是否混用了多种数据结构（保持原有逻辑） ——
    QSet<QString> hits;
    auto hitIf = [&](const QString& key, std::initializer_list<QString> kws){
        for (const auto& k : kws) {
            if (low.contains(k)) {
                hits.insert(key);
                break;
            }
        }
    };
    hitIf("seq",  {"顺序表","顺序","数组","seqlist","seq"});
    hitIf("link", {"链表","链","linklist","link"});
    hitIf("stack",{"栈","stack"});
    hitIf("bt",   {"二叉树","普通二叉树","binary tree","bt"});
    hitIf("bst",  {"二叉搜索树","binary search tree","bst"});
    hitIf("huff", {"哈夫曼","huffman","huff"});
    hitIf("avl",  {"平衡二叉树","avl"});

    if (hits.size() > 1) {
        // 拼接提示用的家族名
        QStringList fam;
        for (const auto& s : hits) fam << s;
        QMessageBox::warning(this, QStringLiteral("输入不合法"),
            QStringLiteral("NLI：同一条指令内只能包含一种数据结构（检测到：%1），请重新输入。")
                .arg(fam.join(", ")));
        return;
    }
    if (hits.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("未识别"),
            QStringLiteral("NLI：未能识别数据结构类型，请补充如“顺序表/链表/栈/二叉树/BST/哈夫曼/AVL”等关键词。"));
        return;
    }
    const QString kind = *hits.begin();

    // ================== 按“然后/接着/之后/并且/同时/句号”等拆成多个子句 ==================
    QString normalized = low;

    // 这些词一般用来串联多个操作；替换成句号，便于 split
    const QStringList connectors = {
        QStringLiteral("然后"),
        QStringLiteral("接着"),
        QStringLiteral("之后"),
        QStringLiteral("并"),
        QStringLiteral("并且"),
        QStringLiteral("最后"),
        QStringLiteral("同时")
    };
    for (const QString& w : connectors) {
        normalized.replace(w, QStringLiteral("。"));
    }

    // 按句号、问号、感叹号、分号和换行分割；注意不要用“，”以免把数字拆散
    QStringList clauses = normalized.split(
        QRegularExpression("[。！？;；\\n]+"),
        Qt::SkipEmptyParts
    );

    // 工具：把整数列表拼成 "1 2 3" 的形式
    auto joinNums = [&](const QVector<int>& a){
        QString s;
        for (int i = 0; i < a.size(); ++i) {
            if (i) s += ' ';
            s += QString::number(a[i]);
        }
        return s;
    };

    QStringList dslLines;   // 多个子句生成的多行 DSL

    // ================== 对每个子句单独生成一条 DSL ==================
    for (QString clause : clauses) {
        clause = clause.trimmed();
        if (clause.isEmpty())
            continue;

        // 只在当前子句中找关键字和数字
        const QVector<int> nums = parseIntList(clause);
        auto hasAny = [&](std::initializer_list<QString> kws)->bool{
            for (const auto& k : kws)
                if (clause.contains(k))
                    return true;
            return false;
        };

        QString dsl;  // 当前子句对应的一条 DSL

        if (kind == "seq") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "seq.clear";
            } else if (hasAny({"插入","插","增","insert"})) {
                if (nums.size() >= 2)
                    dsl = QString("seq.insert %1 %2").arg(nums[0]).arg(nums[1]);
            } else if (hasAny({"删除","删","移除","erase","remove"})) {
                if (nums.size() >= 1)
                    dsl = QString("seq.erase %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "seq " + joinNums(nums);
            }
        }
        else if (kind == "link") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "link.clear";
            } else if (hasAny({"插入","插","增","insert"})) {
                if (nums.size() >= 2)
                    dsl = QString("link.insert %1 %2").arg(nums[0]).arg(nums[1]);
            } else if (hasAny({"删除","删","移除","erase","remove"})) {
                if (nums.size() >= 1)
                    dsl = QString("link.erase %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "link " + joinNums(nums);
            }
        }
        else if (kind == "stack") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "stack.clear";
            } else if (hasAny({"出栈","弹栈","pop"})) {
                dsl = "stack.pop";
            } else if (hasAny({"入栈","压栈","push","加入","添加","增"})) {
                if (nums.size() >= 1)
                    dsl = QString("stack.push %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "stack " + joinNums(nums);
            }
        }
        else if (kind == "bt") {
            if (hasAny({"先序","前序","preorder"})) {
                dsl = "bt.preorder";
            } else if (hasAny({"中序","inorder"})) {
                dsl = "bt.inorder";
            } else if (hasAny({"后序","postorder"})) {
                dsl = "bt.postorder";
            } else if (hasAny({"层序","层次","广度","levelorder"})) {
                dsl = "bt.levelorder";
            } else if (hasAny({"清空","清除","clear"})) {
                dsl = "bt.clear";
            } else if (!nums.isEmpty()) {
                // 默认哨兵 -1（与 BT DSL 约定保持一致）
                dsl = "bt " + joinNums(nums) + " null=-1";
            }
        }
        else if (kind == "bst") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "bst.clear";
            } else if (hasAny({"查找","寻找","搜索","find","search"})) {
                if (nums.size() >= 1)
                    dsl = QString("bst.find %1").arg(nums[0]);
            } else if (hasAny({"插入","插","加入","添加","insert","add"})) {
                if (nums.size() >= 1)
                    dsl = QString("bst.insert %1").arg(nums[0]);
            } else if (hasAny({"删除","删","移除","erase","remove"})) {
                if (nums.size() >= 1)
                    dsl = QString("bst.erase %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "bst " + joinNums(nums);
            }
        }
        else if (kind == "huff") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "huff.clear";
            } else if (!nums.isEmpty()) {
                dsl = "huff " + joinNums(nums);
            }
        }
        else if (kind == "avl") {
            if (hasAny({"清空","清除","clear"})) {
                dsl = "avl.clear";
            } else if (hasAny({"插入","插","加入","添加","insert","add"})) {
                if (nums.size() >= 1)
                    dsl = QString("avl.insert %1").arg(nums[0]);
            } else if (!nums.isEmpty()) {
                dsl = "avl " + joinNums(nums);
            }
        }

        if (!dsl.isEmpty())
            dslLines << dsl;
    }

    // 如果所有子句都没生成有效 DSL，则给出提示
    if (dslLines.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("信息不足"),
            QStringLiteral("NLI：无法从该句生成 DSL，请补充必要的信息（例如位置/值/遍历方式等）。"));
        return;
    }

    // 把多条 DSL 写回编辑框并执行（每条一行，由 runDSL 负责统一校验和动画）
    QString allDsl = dslLines.join("\n");
    dslEdit->setPlainText(allDsl);
    statusBar()->showMessage(QStringLiteral("NLI → DSL：%1").arg(allDsl.replace('\n'," | ")));
    runDSL();
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