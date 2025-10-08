//
// Created by xiang on 25-9-28.
//

#ifndef CANVAS_H
#define CANVAS_H

#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QTimer>

class Canvas : public QGraphicsView {
    Q_OBJECT
    public:
        explicit Canvas(QWidget* parent=nullptr);
        QGraphicsScene* Scene() const { return scene; }

        // 清理与渐入动画（简单好看）
        void resetScene();
        void setTitle(const QString& t);

        // 画基本元素
        QGraphicsEllipseItem* addNode(qreal x, qreal y, const QString& text, bool highlight=false);
        void addEdge(QPointF a, QPointF b);

    private:
        QGraphicsScene* scene;
        QGraphicsTextItem* title;
};

#endif //CANVAS_H
