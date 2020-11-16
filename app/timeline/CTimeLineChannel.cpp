#include "CTimeLineChannel.h"
#include <QGraphicsScene>
#include <QPen>
#include <QPainter>
#include <QLine>
#include <QDebug>

CTimeLineChannel::CTimeLineChannel(const QString &uuid, const QString &label, ITimeLineTrackView *timeline, QObject *parent )
    : ITimeLineChannel( parent )
    , m_uuid( uuid )
    , m_label( label )
    , m_TimeLinePtr( timeline )
{
   setCacheMode( NoCache );
}

QRectF CTimeLineChannel::boundingRect() const
{
    return QRectF(0,0, scene()->width( ), m_TimeLinePtr ? m_TimeLinePtr->channelHeight() : 20);
}

void CTimeLineChannel::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
   if ( m_TimeLinePtr )
   {
      QPen pen;
      pen.setColor( QColorConstants::Gray );
      pen.setWidth(1);
      painter->setPen( pen );

      painter->drawRect( 0, 0, scene()->width()-cFieldMargin, m_TimeLinePtr->channelHeight() );

      painter->setBrush( QBrush( m_labelColor ) );

      painter->drawRect( 0, 0, m_TimeLinePtr->channelLabelWidth() - cFieldMargin, m_TimeLinePtr->channelHeight() );

      QFont font = scene()->font();
      QFontMetricsF fontMetrics( font );

      pen.setColor( ~m_labelColor.rgb() );
      painter->setPen( pen );

      int heightFont = fontMetrics.boundingRect( m_label ).height();
      painter->drawText( 5, m_TimeLinePtr->channelHeight()/2 + heightFont/2, m_label );

   }
}

QColor CTimeLineChannel::color() const
{
   return m_labelColor;
}

void CTimeLineChannel::updateChannelGraphics()
{
   updateEffectPositions();
   update(boundingRect());
}

QVariant CTimeLineChannel::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
   switch ( change )
   {
   case QGraphicsItem::ItemChildAddedChange:
   {
      auto val = value.value<QGraphicsItem*>();
      if ( nullptr != val )
      {
         if ( auto effect = dynamic_cast< IEffect* >( val ) )
         {
            emit effectAdded( this, effect );
         }
      }
      break;
   }

   case QGraphicsItem::ItemChildRemovedChange:
   {
      auto val = value.value<QGraphicsItem*>();
      if ( nullptr != val )
      {
         if ( auto effect = dynamic_cast< IEffect* >( val ) )
         {
            emit effectRemoved( this, effect->getUuid() );
         }
      }
      break;
   }
   default:
      break;
   }

   return ITimeLineChannel::itemChange( change, value);
}
