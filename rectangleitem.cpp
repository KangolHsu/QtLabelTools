#include "rectangleitem.h"

RectangleItem::RectangleItem(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent)
{
    setPen(QPen(Qt::red, 2));
    setBrush(QColor(255, 0, 0, 70)); // Semi-transparent red fill
}

void RectangleItem::setLabel(const QString &label)
{
    itemLabel = label;
    update();
}

QString RectangleItem::getLabel() const
{
    return itemLabel;
}

void RectangleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);

    painter->setPen(Qt::black);
    painter->setBackground(QColor(255, 255, 255, 180));
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->drawText(boundingRect().topLeft() + QPointF(5, 20), itemLabel);
}
