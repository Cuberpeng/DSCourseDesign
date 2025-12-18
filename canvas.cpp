//
// Created by xiang on 25-9-28.
//

#include "canvas.h"
#include <QGraphicsLineItem>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScrollBar>
#include <QSettings>


Canvas::Canvas(QWidget* parent)
    : QGraphicsView(parent),
      scene(new QGraphicsScene(this)) {

    initDefaultColors();

    setScene(scene);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);//抗锯齿
    setBackgroundBrush(QColor("#f7f9fb"));//背景色
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);//每次刷新都更新整个视口
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);//缩放时以鼠标位置为锚点
    setDragMode(QGraphicsView::ScrollHandDrag);

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

// ================= 配色 =================
QString Canvas::normFamily(const QString& family) {
    const QString f = family.trimmed().toLower();
    if (f.isEmpty()) return QStringLiteral("default");
    return f;
}

QStringList Canvas::supportedFamilies() {
    return {
        QStringLiteral("seq"),
        QStringLiteral("link"),
        QStringLiteral("stack"),
        QStringLiteral("bt"),
        QStringLiteral("bst"),
        QStringLiteral("huff"),
        QStringLiteral("avl")
    };
}

void Canvas::initDefaultColors() {
    normalFill_.clear();
    highlightFill_.clear();

    // 顺序表：浅底 + 暖色高亮
    normalFill_[QStringLiteral("seq")]    = QColor("#e8eef9");
    highlightFill_[QStringLiteral("seq")] = QColor("#ffd166");

    // 链表：蓝 + 橙
    normalFill_[QStringLiteral("link")]    = QColor("#3b82f6");
    highlightFill_[QStringLiteral("link")] = QColor("#f59e0b");

    // 栈：蓝系
    normalFill_[QStringLiteral("stack")]    = QColor("#93c5fd");
    highlightFill_[QStringLiteral("stack")] = QColor("#60a5fa");

    // 树：默认蓝/橙（用户可分别调）
    normalFill_[QStringLiteral("bt")]    = QColor("#3b82f6");
    highlightFill_[QStringLiteral("bt")] = QColor("#f59e0b");

    normalFill_[QStringLiteral("bst")]    = QColor("#3b82f6");
    highlightFill_[QStringLiteral("bst")] = QColor("#f59e0b");

    normalFill_[QStringLiteral("huff")]    = QColor("#3b82f6");
    highlightFill_[QStringLiteral("huff")] = QColor("#f59e0b");

    normalFill_[QStringLiteral("avl")]    = QColor("#3b82f6");
    highlightFill_[QStringLiteral("avl")] = QColor("#f59e0b");

    // fallback
    normalFill_[QStringLiteral("default")]   = QColor("#3b82f6");
    highlightFill_[QStringLiteral("default")] = QColor("#f59e0b");
}

void Canvas::loadColors() {
    QSettings s(QStringLiteral("DSCourseDesign"), QStringLiteral("DSCourseDesign"));

    auto loadOne = [&](const QString& fam) {
        const QString kN = QStringLiteral("colors/%1/normal").arg(fam);
        const QString kH = QStringLiteral("colors/%1/highlight").arg(fam);
        if (s.contains(kN)) {
            const QColor c(s.value(kN).toString());
            if (c.isValid()) normalFill_[fam] = c;
        }
        if (s.contains(kH)) {
            const QColor c(s.value(kH).toString());
            if (c.isValid()) highlightFill_[fam] = c;
        }
    };

    loadOne(QStringLiteral("default"));
    for (const auto& f : supportedFamilies()) loadOne(f);
}

void Canvas::saveColors() const {
    QSettings s(QStringLiteral("DSCourseDesign"), QStringLiteral("DSCourseDesign"));

    auto saveOne = [&](const QString& fam) {
        const QString kN = QStringLiteral("colors/%1/normal").arg(fam);
        const QString kH = QStringLiteral("colors/%1/highlight").arg(fam);
        s.setValue(kN, normalFill_.value(fam).name(QColor::HexRgb));
        s.setValue(kH, highlightFill_.value(fam).name(QColor::HexRgb));
    };

    saveOne(QStringLiteral("default"));
    for (const auto& f : supportedFamilies()) saveOne(f);
}

QColor Canvas::deriveBorder(const QColor& fill) {
    return fill.darker(140);
}

QColor Canvas::deriveText(const QColor& fill) {
    const int lum = (fill.red()*299 + fill.green()*587 + fill.blue()*114) / 1000;
    return (lum < 140) ? QColor(Qt::white) : QColor(Qt::black);
}

void Canvas::setCurrentFamily(const QString& family) {
    familyKey_ = normFamily(family);
    if (!normalFill_.contains(familyKey_)) familyKey_ = QStringLiteral("default");
}

void Canvas::setFamilyColors(const QString& family, const QColor& normalFill, const QColor& highlightFill) {
    const QString fam = normFamily(family);
    if (normalFill.isValid())    normalFill_[fam] = normalFill;
    if (highlightFill.isValid()) highlightFill_[fam] = highlightFill;
}

QColor Canvas::familyFillColor(const QString& family, bool highlight) const {
    const QString fam = normFamily(family);
    if (highlight) {
        return highlightFill_.value(fam, highlightFill_.value(QStringLiteral("default"), QColor("#f59e0b")));
    }
    return normalFill_.value(fam, normalFill_.value(QStringLiteral("default"), QColor("#3b82f6")));
}

QBrush Canvas::elementBrush(bool highlight) const {
    return QBrush(familyFillColor(familyKey_, highlight));
}

QPen Canvas::elementPen(bool highlight, qreal width) const {
    QColor fill = familyFillColor(familyKey_, highlight);
    QPen pen(deriveBorder(fill), width);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    return pen;
}

void Canvas::addNode(qreal x, qreal y, const QString &text, bool highlight){
    // 改进的节点样式
    const QColor fill = familyFillColor(familyKey_, highlight);
    const QColor border = deriveBorder(fill);
    // 添加椭圆图元
    auto* n = scene->addEllipse(QRectF(x-35, y-35, 70, 70), QPen(border, 3), QBrush(fill));
    // 添加文本
    auto* label = scene->addText(text);
    label->setDefaultTextColor(deriveText(fill));
    label->setFont(QFont("Arial", 12, QFont::Bold));

    QRectF tb = label->boundingRect();
    label->setPos(x - tb.width()/2, y - tb.height()/2);
    // 改进的阴影效果
    auto* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(15);//阴影模糊半径
    effect->setOffset(3, 3);//阴影偏移
    effect->setColor(QColor(0, 0, 0, 80));//阴影颜色
    n->setGraphicsEffect(effect);// 把阴影效果挂到圆形图元 n
}

void Canvas::addEdge(QPointF a, QPointF b){
    // 实线、线头圆角、折线拐角圆角
    QPen pen(QColor("#678"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    // 主体线段
    scene->addLine(QLineF(a, b), pen);

    // 箭头三角
    const qreal L = 10.0;
    const qreal alpha = M_PI/6.0; // 30°
    qreal ang = std::atan2(b.y() - a.y(), b.x() - a.x());
    // 箭头三角形的两个底角点 p1/p2
    QPointF p1 = b - QPointF(L * std::cos(ang - alpha), L * std::sin(ang - alpha));
    QPointF p2 = b - QPointF(L * std::cos(ang + alpha), L * std::sin(ang + alpha));
    QPolygonF tri; tri << b << p1 << p2;// 用三个点组成一个三角形：顶点是 b，另外两个点是 p1、p2
    scene->addPolygon(tri, pen, QBrush(QColor("#678")));
}
void Canvas::addCurveArrow(QPointF s, QPointF c1, QPointF c2, QPointF e){
    QPen pen(QColor("#678"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QPainterPath path(s);// 创建一条路径，并把起点设为 s
    path.cubicTo(c1, c2, e);// 添加三次贝塞尔曲线：起点 s，控制点 c1/c2，终点 e
    scene->addPath(path, pen);

    // 末端切向方向由 e - c2 决定
    const qreal L = 10.0;
    const qreal alpha = M_PI/6.0;
    qreal ang = std::atan2(e.y() - c2.y(), e.x() - c2.x());// 用终点的切线方向确定方向
    // 计算箭头三角形底边两个点
    QPointF p1 = e - QPointF(L * std::cos(ang - alpha), L * std::sin(ang - alpha));
    QPointF p2 = e - QPointF(L * std::cos(ang + alpha), L * std::sin(ang + alpha));
    QPolygonF tri; tri << e << p1 << p2;// 组装三角形并绘制
    scene->addPolygon(tri, pen, QBrush(QColor("#678")));
}

void Canvas::addBox(qreal x, qreal y, qreal w, qreal h, const QString &text, bool highlight){
    const QColor fill = familyFillColor(familyKey_, highlight);
    const QColor border = deriveBorder(fill);

    QPen pen(border, 2);
    QBrush brush(fill);
    auto* r = scene->addRect(QRectF(x, y, w, h), pen, brush);//矩形图元

    auto* label = scene->addText(text);
    label->setDefaultTextColor(deriveText(fill));
    QRectF tb = label->boundingRect();// 取文本边界，用于居中
    label->setPos(x + (w - tb.width())/2, y + (h - tb.height())/2 - 1);// 把文本放到矩形内部居中位置；y 方向额外 -1 做细微视觉校正

    // 给矩形加一个轻微阴影
    auto* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(6);
    effect->setOffset(0,1);
    r->setGraphicsEffect(effect);
}

// ========== 缩放 ==========
void Canvas::applyZoom(qreal newZoom, const QPointF& anchorViewPos) {
    if (newZoom < minZoom) newZoom = minZoom;
    if (newZoom > maxZoom) newZoom = maxZoom;

    // 以鼠标位置为锚点缩放
    QPointF scenePos = mapToScene(anchorViewPos.toPoint());
    qreal factor = newZoom / currentZoom;// 缩放的倍数
    currentZoom = newZoom;// 更新倍率
    scale(factor, factor);// 对视图做缩放变换

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
    // 场景里没有任何图元就不用 fit，否则后面算 boundingRect 没意义
    if (scene->items().isEmpty()) return;
    QRectF br = scene->itemsBoundingRect().marginsAdded(QMarginsF(40,40,40,40));
    if (br.isEmpty())
        return;

    // 将 fit 作为新的"基准变换"，并把 currentZoom 归一
    resetTransform();
    fitInView(br, Qt::KeepAspectRatio);
    currentZoom = 1.0;
}

// ========== 交互：Ctrl+滚轮缩放 ==========
void Canvas::wheelEvent(QWheelEvent* e) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        const QPoint numDeg = e->angleDelta();// 滚轮转动的角度增量
        if (!numDeg.isNull()) {
            qreal step = (numDeg.y() > 0) ? 1.15 : (1.0/1.15);
            applyZoom(currentZoom * step, e->position());
            e->accept();
            return;
        }
    }
    QGraphicsView::wheelEvent(e); // 正常滚动
}