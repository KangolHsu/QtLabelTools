
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
class QGraphicsLineItem;

class CanvasScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { NoMode, DrawPolygon };
    explicit CanvasScene(QObject *parent = nullptr);

    void setMode(Mode mode);
    void setCurrentLabel(const QString& label);

signals:
    void polygonFinished(PolygonItem* item);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    Mode currentMode = NoMode;
    QString currentLabel;
    QPolygonF currentPolygon;
    QGraphicsPolygonItem* tempPolygonItem = nullptr;
    QGraphicsLineItem* rubberBandLine = nullptr;
};

#endif // CANVASCENE_H