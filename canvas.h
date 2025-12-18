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
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QStringList>
#include <QHash>

class Canvas : public QGraphicsView {
    Q_OBJECT
public:
    explicit Canvas(QWidget* parent=nullptr);
    QGraphicsScene* Scene() const { return scene; }

    // 清理与标题
    void resetScene();
    void setTitle(const QString& t);

    // ================= 配色：按“数据结构类型”区分（普通/高亮） =================
    // family 约定："seq" "link" "stack" "bt" "bst" "huff" "avl"
    void setCurrentFamily(const QString& family);
    QString currentFamily() const { return familyKey_; }

    void setFamilyColors(const QString& family, const QColor& normalFill, const QColor& highlightFill);
    QColor familyFillColor(const QString& family, bool highlight) const;

    // 当前 family 的画笔/画刷（给 MainWindow 里那些直接 Scene()->addRect 的地方复用）
    QBrush elementBrush(bool highlight) const;
    QPen   elementPen(bool highlight, qreal width = 2.0) const;

    static QStringList supportedFamilies();

    // 画基本元素
    void addNode(qreal x, qreal y, const QString& text, bool highlight=false);
    void addEdge(QPointF a, QPointF b);
    void addCurveArrow(QPointF s, QPointF c1, QPointF c2, QPointF e);
    void addBox(qreal x, qreal y, qreal w, qreal h, const QString& text, bool highlight=false);

    // 缩放
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void zoomFit();

protected:
    void wheelEvent(QWheelEvent* e) override;

private:
    QGraphicsScene* scene{};
    QGraphicsTextItem* title{};
    qreal currentZoom = 1.0;
    const qreal minZoom = 0.05;
    const qreal maxZoom = 8.0;

    void applyZoom(qreal newZoom, const QPointF& anchorViewPos);

    // ===== 配色存储 =====
    QString familyKey_ = QStringLiteral("default");
    QHash<QString, QColor> normalFill_;
    QHash<QString, QColor> highlightFill_;

    void initDefaultColors();
    void loadColors();
    void saveColors() const;

    static QColor deriveBorder(const QColor& fill);
    static QColor deriveText(const QColor& fill);
    static QString normFamily(const QString& family);
};

#endif // CANVAS_H

