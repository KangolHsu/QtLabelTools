/* *************************************************************** */
/* canvasview.cpp                          */
/* *************************************************************** */
#include "canvasview.h"
#include <QWheelEvent>

CanvasView::CanvasView(QWidget *parent) : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void CanvasView::wheelEvent(QWheelEvent *event)
{
    qreal scaleFactor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    scale(scaleFactor, scaleFactor);
}

void CanvasView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    if (scene()) {
        scene()->update();
    }

    
}
