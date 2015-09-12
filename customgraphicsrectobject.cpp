#include "customgraphicsrectobject.h"

#include <QGraphicsSceneMouseEvent>

customGraphicsRectObject::customGraphicsRectObject(QRect rect, QColor color, ruleCluster *cptr)
{
    bRect = rect;
    rectColor = color;
    c = *cptr;

    isHoverOn = false;

    setAcceptHoverEvents(true);
}

QRectF customGraphicsRectObject::boundingRect() const
{
    return QRectF(bRect);
}

void customGraphicsRectObject::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rec = boundingRect();
    QBrush rectBrush(Qt::blue);
    QString text = "";

    if(c.rule!="")
        text += "R";
    else
        text+= "J";

    text += QString::number(c.clusterID+1);

    if(isHoverOn)
        rectBrush.setColor(Qt::red);
    else
        rectBrush.setColor(rectColor);

    painter->fillRect(rec,rectBrush);
    painter->drawRect(rec);
    painter->drawText(rec,Qt::AlignCenter,text);
}

customGraphicsRectObject::~customGraphicsRectObject()
{

}

void customGraphicsRectObject::hoverEnterEvent(QGraphicsSceneHoverEvent* h)
{
    isHoverOn = true;
    update();
    QGraphicsItem::hoverEnterEvent(h);
}

void customGraphicsRectObject::hoverLeaveEvent(QGraphicsSceneHoverEvent* h)
{
    isHoverOn = false;
    update();
    QGraphicsItem::hoverLeaveEvent(h);
}

void customGraphicsRectObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

