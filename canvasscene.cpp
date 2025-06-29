/* *************************************************************** */
/* canvasscene.cpp                         */
/* *************************************************************** */
#include "canvasscene.h"
#include "polygonitem.h"
#include "rectangleitem.h"
#include "mainwindow.h"
#include <QGraphicsLineItem>
#include <QPen>
#include <QMenu>
#include <QKeyEvent>

CanvasScene::CanvasScene(QObject *parent) : QGraphicsScene(parent) {}

void CanvasScene::setMode(Mode mode)
{
    currentMode = mode;
    if (mode == NoMode) {
        if (tempPolygonItem) {
            removeItem(tempPolygonItem);
            delete tempPolygonItem;
            tempPolygonItem = nullptr;
        }
        if (rubberBandLine) {
            removeItem(rubberBandLine);
            delete rubberBandLine;
            rubberBandLine = nullptr;
        }
        if (currentItem) {
            removeItem(currentItem);
            delete currentItem;
            currentItem = nullptr;
        }
        currentPolygon.clear();
    }
}

void CanvasScene::setCurrentLabel(const QString &label)
{
    currentLabel = label;
}

void CanvasScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    if (currentMode == DrawPolygon) {
        QPointF point = event->scenePos();
        currentPolygon << point;

        if (tempPolygonItem) {
            tempPolygonItem->setPolygon(currentPolygon);
        } else {
            tempPolygonItem = addPolygon(currentPolygon, QPen(Qt::blue, 2));
        }
        
        if (!rubberBandLine) {
            rubberBandLine = addLine(QLineF(point, point), QPen(Qt::DashLine));
        }
        rubberBandLine->setLine(QLineF(point, point));
    } else if (currentMode == DrawRectangle) {
        startPoint = event->scenePos();
        currentItem = new RectangleItem(QRectF(startPoint, startPoint));
        addItem(currentItem);
    } else {
        QGraphicsScene::mousePressEvent(event);
    }
}

void CanvasScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (currentMode == DrawPolygon && !currentPolygon.isEmpty() && rubberBandLine) {
        rubberBandLine->setLine(QLineF(currentPolygon.last(), event->scenePos()));
    } else if (currentMode == DrawRectangle && currentItem) {
        auto rectItem = static_cast<RectangleItem*>(currentItem);
        QRectF newRect(startPoint, event->scenePos());
        rectItem->setRect(newRect.normalized());
    } else {
        QGraphicsScene::mouseMoveEvent(event);
    }
}

void CanvasScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsScene::mouseReleaseEvent(event);
        return;
    }

    if (currentMode == DrawRectangle && currentItem) {
        auto rectItem = static_cast<RectangleItem*>(currentItem);
        rectItem->setLabel(currentLabel);
        rectItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
        emit rectangleFinished(rectItem);
        currentItem = nullptr; // The item is now permanent
    } else {
        QGraphicsScene::mouseReleaseEvent(event);
    }
}

void CanvasScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (currentMode == DrawPolygon && currentPolygon.size() > 2) {
        if (tempPolygonItem) {
            removeItem(tempPolygonItem);
            delete tempPolygonItem;
            tempPolygonItem = nullptr;
        }
        if (rubberBandLine) {
            removeItem(rubberBandLine);
            delete rubberBandLine;
            rubberBandLine = nullptr;
        }

        auto finalPolygonItem = new PolygonItem(currentPolygon);
        finalPolygonItem->setLabel(currentLabel);
        finalPolygonItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
        addItem(finalPolygonItem);
        
        currentPolygon.clear();
        emit polygonFinished(finalPolygonItem);
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

void CanvasScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (currentMode == DrawRectangle && currentItem) {
            removeItem(currentItem);
            delete currentItem;
            currentItem = nullptr;
            setMode(NoMode);
        } else if (currentMode == DrawPolygon) {
            // Handle polygon cancellation if needed
            setMode(NoMode);
        }
    }
    QGraphicsScene::keyPressEvent(event);
}


void CanvasScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
    if (item && (dynamic_cast<PolygonItem*>(item) || dynamic_cast<RectangleItem*>(item))) {
        clearSelection();
        item->setSelected(true);

        QMenu menu;
        QAction *changeLabelAction = menu.addAction("修改标签");
        QAction *deleteAction = menu.addAction("删除标注");

        QAction *selectedAction = menu.exec(event->screenPos());

        MainWindow* mw = qobject_cast<MainWindow*>(parent());

        if (mw) {
            if (selectedAction == deleteAction) {
                mw->deleteSelectedShape();
            } else if (selectedAction == changeLabelAction) {
                mw->changeSelectedShapeLabel();
            }
        }
    }
    QGraphicsScene::contextMenuEvent(event);
}
