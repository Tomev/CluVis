#include "customgraphicellipseobject.h"

#include <QGraphicsSceneMouseEvent>

customGraphicEllipseObject::customGraphicEllipseObject(QRect rect, QColor color, ruleCluster *cptr)
{
    bRect = rect;
    rectColor = color;
    c = *cptr;

    isHoverOn = false;

    setAcceptHoverEvents(true);
}

QRectF customGraphicEllipseObject::boundingRect() const
{
    return QRectF(bRect);
}

void customGraphicEllipseObject::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rec = boundingRect();
    QBrush rectBrush(Qt::blue);
    QString text = c.name();

    if(isHoverOn)
        rectBrush.setColor(Qt::red);
    else
        rectBrush.setColor(rectColor);

    painter->setBrush(rectBrush);
    painter->drawEllipse(rec);
    painter->drawText(rec,Qt::AlignCenter,text);

}

customGraphicEllipseObject::~customGraphicEllipseObject()
{

}

void customGraphicEllipseObject::hoverEnterEvent(QGraphicsSceneHoverEvent* h)
{
    isHoverOn = true;
    update();
    QGraphicsItem::hoverEnterEvent(h);
}

void customGraphicEllipseObject::hoverLeaveEvent(QGraphicsSceneHoverEvent* h)
{
    isHoverOn = false;
    update();
    QGraphicsItem::hoverLeaveEvent(h);
}

void customGraphicEllipseObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        emit getClusterInfo(&c);
    }
    else
    {
        emit ruleClusterPicked(&c);
    }
}
