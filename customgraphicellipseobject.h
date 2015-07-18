#ifndef CUSTOMGRAPHICELLIPSEOBJECT_H
#define CUSTOMGRAPHICELLIPSEOBJECT_H

#include <QGraphicsObject>
#include <QPainter>
#include <QDebug>

#include "clusters.h"

class customGraphicEllipseObject : public QGraphicsObject
{
    Q_OBJECT

public:
    customGraphicEllipseObject(QRect rect, QColor color, ruleCluster* cptr);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);


    customGraphicEllipseObject();
    ~customGraphicEllipseObject();

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

#endif // CUSTOMGRAPHICELLIPSEOBJECT_H
