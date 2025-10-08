//
// Created by xiang on 25-10-7.
//

#ifndef TOOL_H
#define TOOL_H
#include <QString>
#include <QPointF>

//inline QString four(int v){ return QString::number(v); }
inline QPointF offset(QPointF p, qreal dx, qreal dy) {
    return QPointF(p.x()+dx, p.y()+dy);
}


#endif //TOOL_H
