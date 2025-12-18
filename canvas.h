//
// Created by xiang on 25-9-28.
//

#ifndef CANVAS_H
#define CANVAS_H

#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>

class Canvas : public QGraphicsView {
    Q_OBJECT
public:
    explicit Canvas(QWidget* parent=nullptr);
    QGraphicsScene* Scene() const { return scene; }

    // 清理与标题
    void resetScene();
    void setTitle(const QString& t);

    // 画基本元素
    void addNode(qreal x, qreal y, const QString &text, bool highlight = false);
    void addEdge(QPointF a, QPointF b);
    void addCurveArrow(QPointF s, QPointF c1, QPointF c2, QPointF e);

    void addBox(qreal x, qreal y, qreal w, qreal h, const QString &text, bool highlight = false);

    // 缩放控制（供工具栏按钮调用）
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void zoomFit();

protected:
    // 交互：Ctrl+滚轮缩放、空格/中键拖拽
    void wheelEvent(QWheelEvent* e) override;

private:
    QGraphicsScene* scene{};
    QGraphicsTextItem* title{};
    qreal currentZoom = 1.0;               // 相对基准比例
    const qreal minZoom = 0.05;
    const qreal maxZoom = 8.0;
    bool spacePanning = false;
    bool middlePanning = false;

    void applyZoom(qreal newZoom, const QPointF& anchorViewPos);
};

#endif // CANVAS_H

