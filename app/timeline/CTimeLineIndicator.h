#pragma once

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QLine>
#include <QPolygonF>
#include <QVector>
#include <QPointF>
#include <QGraphicsSceneMouseEvent>
#include "ITimeLineTrackView.h"

class CTimeLineIndicator: public QGraphicsItem
{
public:
    CTimeLineIndicator();

    // QGraphicsItem interface
public:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // QGraphicsItem interface
    ITimeLineTrackView *TimeLinePtr() const;
    void setTimeLinePtr(ITimeLineTrackView *TimeLinePtr);

    void setPosition( int64_t position );

    void updateScenePosition();

    int64_t position() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    // QGraphicsItem interface
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QSizeF calculateSize() const;
    void setHeight(int height){line.setP2(QPoint(0,height));}

    QVector<QPointF> points;
    QBrush brush;
    QPen pen;

    QLine line;
    QPolygonF poly;
    bool pressed = false;
    ITimeLineTrackView* m_TimeLinePtr = nullptr;
    int64_t m_position = 0;

};
