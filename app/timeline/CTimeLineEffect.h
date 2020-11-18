#pragma once

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "ITimeLineTrackView.h"


class CTimeLineEffect
        : public IEffect
{
public:
    CTimeLineEffect( ITimeLineChannel *channel, std::shared_ptr<IEffectGenerator> effectGenerator,
                    uint64_t position,
                    QObject* parent = nullptr );

    CTimeLineEffect( ITimeLineChannel *channel, std::shared_ptr<IEffectGenerator> effectGenerator,
                    QObject* parent = nullptr );

    // QGraphicsItem interface
public:
    virtual QRectF boundingRect( ) const override;
    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) override;

    // QGraphicsItem interface
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    // QGraphicsItem interface
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool pressedForMove=false;
    bool pressedForChangeDuration=false;
    QPointF oldPos;
    QPointF oldMousePos;

};

