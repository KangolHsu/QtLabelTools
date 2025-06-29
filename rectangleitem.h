#ifndef RECTANGLEITEM_H
#define RECTANGLEITEM_H

#include <QGraphicsRectItem>
#include <QPainter>

class RectangleItem : public QGraphicsRectItem
{
public:
    RectangleItem(const QRectF &rect, QGraphicsItem *parent = nullptr);

    void setLabel(const QString &label);
    QString getLabel() const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QString itemLabel;
};

#endif // RECTANGLEITEM_H
