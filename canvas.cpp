//
// Created by xiang on 25-9-28.
//
#include "canvas.h"
#include <QGraphicsLineItem>
#include <QGraphicsDropShadowEffect>

Canvas::Canvas(QWidget* parent): QGraphicsView(parent), scene(new QGraphicsScene(this)) {
    setScene(scene);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);//抗锯齿
    setBackgroundBrush(QColor("#f7f9fb"));//背景色
    title = scene->addText("");//标题文本
    title->setDefaultTextColor(Qt::darkGray);//文本默认颜色
    title->setPos(10, 5);//标题位置

}

void Canvas::setTitle(const QString& t) {//标题文字
    title->setPlainText(t);
}

void Canvas::resetScene(){//初始化
    scene->clear();
    title = scene->addText("");
    title->setDefaultTextColor(Qt::darkGray);
    title->setPos(10, 5);
}

QGraphicsEllipseItem* Canvas::addNode(qreal x, qreal y, const QString& text, bool highlight){ // 绘制节点
    auto* n=scene->addEllipse(QRectF(x-20, y-20, 40, 40), QPen(Qt::NoPen), QBrush(highlight?QColor("#ffd166"): QColor("#a8dadc"))); // 在场景中添加圆形
    auto* label = scene->addText(text); // 添加标题文本
    label->setDefaultTextColor(Qt::black); // 设置文本默认颜色
    QRectF tb=label->boundingRect();//边界矩形
    label->setPos(x- tb.width()/2, y- tb.height()/2);//设置位置

    auto* effect =new QGraphicsDropShadowEffect;// 添加阴影效果以增强立体感
    effect->setBlurRadius(8);//设置半径
    effect->setOffset(0,2);//设置偏移
    n->setGraphicsEffect(effect);
    return n;
}

void Canvas::addEdge(QPointF a, QPointF b){//绘制边
    scene->addLine(QLineF(a, b), QPen(QColor("#678"), 2));
}