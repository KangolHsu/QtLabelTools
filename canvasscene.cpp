
/* *************************************************************** */
/* canvasscene.cpp                         */
/* *************************************************************** */
#include "canvasscene.h"
#include "polygonitem.h"
#include "mainwindow.h"
#include <QGraphicsLineItem>
#include <QPen>
#include <QMenu>

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
        currentPolygon.clear();
    }
}

void CanvasScene::setCurrentLabel(const QString &label)
{
    currentLabel = label;
}

void CanvasScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (currentMode == DrawPolygon && event->button() == Qt::LeftButton) {
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
    }
    // 将事件传递给场景中的item，以便它们能处理自己的鼠标事件（如拖拽顶点）
    QGraphicsScene::mousePressEvent(event);
}

void CanvasScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (currentMode == DrawPolygon && !currentPolygon.isEmpty() && rubberBandLine) {
        rubberBandLine->setLine(QLineF(currentPolygon.last(), event->scenePos()));
    }
    QGraphicsScene::mouseMoveEvent(event);
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
        setMode(NoMode);
        emit polygonFinished(finalPolygonItem);
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

void CanvasScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // 检查鼠标下是否有我们自定义的多边形项
    QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
    PolygonItem* polygonItem = dynamic_cast<PolygonItem*>(item);

    if (polygonItem) {
        // 如果有，就创建一个右键菜单
        clearSelection();
        polygonItem->setSelected(true);

        QMenu menu;
        QAction *changeLabelAction = menu.addAction("修改标签");
        QAction *deleteAction = menu.addAction("删除标注");

        // 执行菜单，并根据用户的选择执行相应的动作
        QAction *selectedAction = menu.exec(event->screenPos());

        // 获取主窗口指针来调用槽函数
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
