#ifndef CTIMELINECHANNEL_H
#define CTIMELINECHANNEL_H

#include <QObject>
#include <QString>
#include <QColor>
#include "ITimeLineTrackView.h"

class CTimeLineChannel
      : public ITimeLineChannel
{
    Q_OBJECT
public:
    CTimeLineChannel( const QString& uuid, const QString& label, ITimeLineTrackView* timeline, QObject* parent = nullptr );


    const QString& uuid() const override { return m_uuid; }
    void setUuid(const QString &uuid) { m_uuid = uuid; }

    const QString& label() const override { return m_label; }
    void setLabel(const QString &label) { m_label = label; }

    void setTimeLinePtr( ITimeLineTrackView* ptr ) { m_TimeLinePtr = ptr; }
    void setColor( const QColor& color) { m_labelColor = color; }

    // ITimeLineChannel interface
    ITimeLineTrackView* timeLinePtr( ) const override { return m_TimeLinePtr; }
    virtual QColor color() const override;

    void updateChannelGraphics();

    // QGraphicsItem interface

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QString m_uuid;
    QString m_label;
    QColor  m_labelColor;

    ITimeLineTrackView* m_TimeLinePtr = nullptr;

};

#endif // CTIMELINECHANNEL_H
