#ifndef CUSTOMGRAPHICSRECTOBJECT_H
#define CUSTOMGRAPHICSRECTOBJECT_H

#include <QGraphicsObject>
#include <QPainter>
#include <QDebug>

#include "clusters.h"

class customGraphicsRectObject : public QGraphicsObject
{
    Q_OBJECT

public:
    customGraphicsRectObject(QRect rect, QColor color, ruleCluster* cptr);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

    customGraphicsRectObject();
    ~customGraphicsRectObject();

signals:
    void ruleClusterPicked(ruleCluster*);
    void getClusterInfo(ruleCluster*);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* h);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* h);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    QRect bRect;
    QColor rectColor;
    ruleCluster c;

    bool isHoverOn;

};

#endif // CUSTOMGRAPHICSRECTOBJECT_H
