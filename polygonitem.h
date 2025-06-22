/* *************************************************************** */
/* polygonitem.h                         */
/* *************************************************************** */
#ifndef POLYGONITEM_H
#define POLYGONITEM_H

#include <QGraphicsPolygonItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

class PolygonItem : public QGraphicsPolygonItem
{
public:
    explicit PolygonItem(const QPolygonF &polygon, QGraphicsItem *parent = nullptr);

    void setLabel(const QString& label);
    QString getLabel() const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    // 添加鼠标事件来处理顶点拖拽
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString itemLabel;
    
    // 用于顶点编辑
    int vertexSize = 6;
    int draggingVertexIndex = -1; // -1表示没有顶点被拖拽
};

#endif // POLYGONITEM_H