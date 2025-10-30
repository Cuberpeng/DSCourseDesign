//
// Created by xiang on 25-9-28.
//

#include "canvas.h"
#include <QGraphicsLineItem>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScrollBar>

Canvas::Canvas(QWidget* parent)
    : QGraphicsView(parent),
      scene(new QGraphicsScene(this)) {

    setScene(scene);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    setBackgroundBrush(QColor("#f7f9fb"));
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::NoDrag);

    title = scene->addText("");
    title->setDefaultTextColor(Qt::darkGray);
    title->setPos(10, 5);
}

void Canvas::setTitle(const QString& t) {
    if (title) title->setPlainText(t);
}


void Canvas::resetScene(){
    scene->clear();
    title = scene->addText("");
    title->setDefaultTextColor(Qt::darkGray);
    title->setPos(10, 5);
    // 保持当前缩放与视图状态，不强制 resetTransform
}

QGraphicsEllipseItem* Canvas::addNode(qreal x, qreal y, const QString& text, bool highlight){
    auto* n=scene->addEllipse(QRectF(x-33, y-33, 66, 66), QPen(Qt::NoPen), QBrush(highlight ? QColor("#ffd166"): QColor("#a8dadc")));
    auto* label = scene->addText(text);
    label->setDefaultTextColor(Qt::black);
    QRectF tb=label->boundingRect();
    label->setPos(x- tb.width()/2, y- tb.height()/2);

    auto* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(8);
    effect->setOffset(0,2);
    n->setGraphicsEffect(effect);
    return n;
}

void Canvas::addEdge(QPointF a, QPointF b){
    QPen pen(QColor("#678"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    // 主体线段
    scene->addLine(QLineF(a, b), pen);

    // 箭头三角
    const qreal L = 10.0;
    const qreal alpha = M_PI/6.0; // 30°
    qreal ang = std::atan2(b.y() - a.y(), b.x() - a.x());
    QPointF p1 = b - QPointF(L * std::cos(ang - alpha), L * std::sin(ang - alpha));
    QPointF p2 = b - QPointF(L * std::cos(ang + alpha), L * std::sin(ang + alpha));
    QPolygonF tri; tri << b << p1 << p2;
    scene->addPolygon(tri, pen, QBrush(QColor("#678")));
}
void Canvas::addCurveArrow(QPointF s, QPointF c1, QPointF c2, QPointF e){
    QPen pen(QColor("#678"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QPainterPath path(s);
    path.cubicTo(c1, c2, e);
    scene->addPath(path, pen);

    // 末端切向方向由 e - c2 决定
    const qreal L = 10.0;
    const qreal alpha = M_PI/6.0;
    qreal ang = std::atan2(e.y() - c2.y(), e.x() - c2.x());
    QPointF p1 = e - QPointF(L * std::cos(ang - alpha), L * std::sin(ang - alpha));
    QPointF p2 = e - QPointF(L * std::cos(ang + alpha), L * std::sin(ang + alpha));
    QPolygonF tri; tri << e << p1 << p2;
    scene->addPolygon(tri, pen, QBrush(QColor("#678")));
}

QGraphicsRectItem* Canvas::addBox(qreal x, qreal y, qreal w, qreal h,
                                  const QString& text, bool highlight){
    QPen pen(QColor("#5f6c7b"), 2);
    QBrush brush(highlight ? QColor("#ffd166") : QColor("#e8eef9"));
    auto* r = scene->addRect(QRectF(x, y, w, h), pen, brush);

    auto* label = scene->addText(text);
    label->setDefaultTextColor(Qt::black);
    QRectF tb = label->boundingRect();
    label->setPos(x + (w - tb.width())/2, y + (h - tb.height())/2 - 1);

    auto* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(6);
    effect->setOffset(0,1);
    r->setGraphicsEffect(effect);
    return r;
}

// ========== 缩放 ==========
void Canvas::applyZoom(qreal newZoom, const QPointF& anchorViewPos) {
    if (newZoom < minZoom) newZoom = minZoom;
    if (newZoom > maxZoom) newZoom = maxZoom;

    // 以鼠标位置为锚点缩放
    QPointF scenePos = mapToScene(anchorViewPos.toPoint());
    qreal factor = newZoom / currentZoom;
    currentZoom = newZoom;
    scale(factor, factor);

    // 保持锚点位置稳定
    QPointF viewPosAfter = mapFromScene(scenePos);
    QPointF delta = anchorViewPos - viewPosAfter;
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
}

void Canvas::zoomIn() { applyZoom(currentZoom * 1.15, rect().center()); }
void Canvas::zoomOut() { applyZoom(currentZoom / 1.15, rect().center()); }

void Canvas::zoomReset() {
    resetTransform();
    currentZoom = 1.0;
}

void Canvas::zoomFit() {
    if (scene->items().isEmpty()) return;
    QRectF br = scene->itemsBoundingRect().marginsAdded(QMarginsF(40,40,40,40));
    if (br.isEmpty())
        return;

    // 将 fit 作为新的“基准变换”，并把 currentZoom 归一
    resetTransform();
    fitInView(br, Qt::KeepAspectRatio);
    currentZoom = 1.0;
}

// ========== 交互：Ctrl+滚轮、空格/中键拖拽 ==========
void Canvas::wheelEvent(QWheelEvent* e) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        const QPoint numDeg = e->angleDelta();
        if (!numDeg.isNull()) {
            qreal step = (numDeg.y() > 0) ? 1.15 : (1.0/1.15);
            applyZoom(currentZoom * step, e->position());
            e->accept();
            return;
        }
    }
    QGraphicsView::wheelEvent(e); // 正常滚动
}

void Canvas::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::MiddleButton || (e->button()==Qt::LeftButton && (QApplication::keyboardModifiers() & Qt::KeyboardModifier::ShiftModifier))) {
        middlePanning = true;
        setDragMode(QGraphicsView::ScrollHandDrag);
        QGraphicsView::mousePressEvent(e);
        return;
    }
    if (spacePanning && e->button()==Qt::LeftButton) {
        setDragMode(QGraphicsView::ScrollHandDrag);
        QGraphicsView::mousePressEvent(e);
        return;
    }
    QGraphicsView::mousePressEvent(e);
}

void Canvas::mouseReleaseEvent(QMouseEvent* e) {
    if (middlePanning && e->button()==Qt::MiddleButton) {
        middlePanning = false;
        setDragMode(QGraphicsView::NoDrag);
    }
    if (spacePanning && e->button()==Qt::LeftButton) {
        setDragMode(QGraphicsView::NoDrag);
    }
    QGraphicsView::mouseReleaseEvent(e);
}

void Canvas::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Space && !spacePanning) {
        spacePanning = true;
        // 不立即切 NoDrag->HandDrag，等待左键按下时再切
        e->accept();
        return;
    }
    QGraphicsView::keyPressEvent(e);
}

void Canvas::keyReleaseEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Space && spacePanning) {
        spacePanning = false;
        setDragMode(QGraphicsView::NoDrag);
        e->accept();
        return;
    }
    QGraphicsView::keyReleaseEvent(e);
}
