#pragma once

#include <QGraphicsItem>
#include <QPen>
#include <QPainter>
#include <QLine>
#include <QPointF>

#include "ITimeLineTrackView.h"

class CTimeLinePosition: public QGraphicsItem
{
public:
    CTimeLinePosition();

    // QGraphicsItem interface
public:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    ITimeLineTrackView *TimeLinePtr() const;
    void setTimeLinePtr(ITimeLineTrackView *TimeLinePtr);

    void setPosition( int64_t position );

    void updateScenePosition();

    int64_t position() const;

private:
    QPen pen;
    ITimeLineTrackView* m_TimeLinePtr = nullptr;
    int64_t m_position = 0;
};
