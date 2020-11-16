#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include "ITimeLineTrackView.h"
#include "CTimeLineChannel.h"
#include "CTimeLinePosition.h"
#include "CTimeLineIndicator.h"

class CTimeLineView : public QGraphicsView
      , public ITimeLineTrackView
{
    Q_OBJECT
public:
    CTimeLineView(QWidget *parent = nullptr);
    
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    void AddItem(QPointF pos, QRect rect, QPen pen, QBrush brush);

signals:
    void sendMousePressEventSignal(QMouseEvent *event);
    void sendMouseMoveEventSignal(QMouseEvent *event);
    void sendMouseReleaseEventSignal(QMouseEvent *event);
    void playFromPosition(CTimeLineView* view, uint64_t position);

    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    // QGraphicsView interface
protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

private:

    void updateSceneRect();

    // ITimeLineTrackViev interface
public:
    virtual uint32_t channelHeight() const override;
    virtual int64_t compositionDuration() const override;
    virtual uint64_t channelLabelWidth() const override;
    virtual int64_t compositionPosition() const override;

    virtual qreal    convertPositionToSceneX() const override;
    virtual qreal    convertPositionToSceneX( int64_t position ) const override;
    virtual int64_t  convertSceneXToPosition( qreal x ) const override;
    virtual ITimeLineChannel *getNeiborChannel( ITimeLineChannel* channel, int offsetIndex ) const override;

    void addChannel( CTimeLineChannel* channel );
    void removeChannel( CTimeLineChannel* channel );
    void clearChannels();

    void setChannelLabelWidth(const uint32_t &channelLabelWidth);

    uint32_t timeLabelsHeight() const;
    void setTimeLabelsHeight(const uint32_t &tlh);

    float scale() const;
    void setScale(float scale);

    const std::vector<CTimeLineChannel *> &channels() const;

    void setChannelHeight(uint32_t &&ch);

    void setCompositionDuration( int64_t length );
    void setCompositionPosition( int64_t position );

private:
    CTimeLineIndicator* indicator;
    CTimeLinePosition*  indicatorPosition;
    uint32_t m_channelLabelWidth = 200;
    uint32_t m_channelHeight = 20;
    uint32_t m_timeLabelsHeight = 50;
    float    m_scale = 1.0f;
    std::vector< CTimeLineChannel* > m_channels;

    bool m_isAltPressed = false;

    int64_t m_compositionDuration = 100000;
    int64_t m_compositionPosition = 0;

};
