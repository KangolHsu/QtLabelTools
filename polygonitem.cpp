
/* *************************************************************** */
/* polygonitem.cpp                         */
/* *************************************************************** */
#include "polygonitem.h"

PolygonItem::PolygonItem(const QPolygonF &polygon, QGraphicsItem *parent)
    : QGraphicsPolygonItem(polygon, parent)
{
    setPen(QPen(Qt::green, 2));
    setBrush(QColor(0, 255, 0, 70)); // 半透明绿色填充
    setAcceptHoverEvents(true); // 开启Hover事件以检测鼠标靠近顶点
}

void PolygonItem::setLabel(const QString &label)
{
    itemLabel = label;
}

QString PolygonItem::getLabel() const
{
    return itemLabel;
}

void PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 调用基类的paint来绘制多边形本身
    QGraphicsPolygonItem::paint(painter, option, widget);
    
    // 如果item被选中，则绘制顶点控制点
    if (isSelected()) {
        painter->setPen(QPen(Qt::red, 2));
        painter->setBrush(Qt::red);
        for (const QPointF &point : polygon()) {
            painter->drawEllipse(point, vertexSize / 2, vertexSize / 2);
        }
    }

    // 在多边形的左上角绘制标签文本
    painter->setPen(Qt::black);
    painter->setBackground(QColor(255, 255, 255, 180));
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->drawText(boundingRect().topLeft() + QPointF(5, 20), itemLabel);
}

void PolygonItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    bool onVertex = false;
    for (const QPointF &vertex : polygon()) {
        if (QLineF(event->pos(), vertex).length() < vertexSize) {
            onVertex = true;
            break;
        }
    }
    
    if (onVertex) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsItem::hoverMoveEvent(event);
}

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        for (int i = 0; i < polygon().size(); ++i) {
            if (QLineF(event->pos(), polygon().at(i)).length() < vertexSize) {
                draggingVertexIndex = i; // 记录被拖拽的顶点索引
                return;
            }
        }
    }
    draggingVertexIndex = -1;
    // 调用基类方法以支持item本身的拖拽
    QGraphicsItem::mousePressEvent(event);
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (draggingVertexIndex != -1) {
        // 更新多边形形状
        prepareGeometryChange(); // 必须在改变几何形状前调用
        QPolygonF newPolygon = polygon();
        newPolygon[draggingVertexIndex] = event->pos();
        setPolygon(newPolygon);
    } else {
        // 如果没有拖拽顶点，则执行item本身的拖拽
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    draggingVertexIndex = -1; // 释放鼠标，重置拖拽状态
    QGraphicsItem::mouseReleaseEvent(event);
}
