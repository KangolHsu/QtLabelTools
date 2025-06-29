
/* *************************************************************** */
/* canvasscene.h                         */
/* *************************************************************** */
#ifndef CANVASCENE_H
#define CANVASCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPolygonF>
#include <QMenu>

class PolygonItem;
class RectangleItem;
class QGraphicsLineItem;

class CanvasScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { NoMode, DrawPolygon, DrawRectangle };
    explicit CanvasScene(QObject *parent = nullptr);

    void setMode(Mode mode);
    void setCurrentLabel(const QString& label);

signals:
    void polygonFinished(PolygonItem* item);
    void rectangleFinished(RectangleItem* item);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    Mode currentMode = NoMode;
    QString currentLabel;
    QPolygonF currentPolygon;
    QGraphicsPolygonItem* tempPolygonItem = nullptr;
    QGraphicsLineItem* rubberBandLine = nullptr;

    QPointF startPoint;
    QGraphicsItem* currentItem = nullptr; // Generic pointer for the item being drawn
};

#endif // CANVASCENE_H
