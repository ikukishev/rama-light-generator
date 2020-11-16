#include <QGraphicsScene>
#include <QDebug>
#include "CTimeLinePosition.h"

CTimeLinePosition::CTimeLinePosition()
   : QGraphicsItem ()
   , pen( QPen( QColor(255,0,0, 155) , 1) )
{
}

int64_t CTimeLinePosition::position() const
{
   return m_position;
}

ITimeLineTrackView *CTimeLinePosition::TimeLinePtr() const
{
   return m_TimeLinePtr;
}

void CTimeLinePosition::setTimeLinePtr(ITimeLineTrackView *TimeLinePtr)
{
   m_TimeLinePtr = TimeLinePtr;
}

void CTimeLinePosition::setPosition(int64_t position)
{
   if ( position >=0 && nullptr != m_TimeLinePtr )
   {
      if ( m_TimeLinePtr->compositionDuration() > position )
      {
         m_position = position;
         updateScenePosition();
      }
   }
}

void CTimeLinePosition::updateScenePosition()
{
   if ( nullptr != m_TimeLinePtr )
   {
      setX( m_TimeLinePtr->convertPositionToSceneX( m_position ) );
   }
}

QRectF CTimeLinePosition::boundingRect() const
{
    return QRectF(0,0,pen.width(), scene() ? scene()->height() : 0);
}

void CTimeLinePosition::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
   if ( scene() )
   {
      updateScenePosition();
      painter->setPen(pen);
      painter->drawLine( QLine( 0, 0, 0, scene()->height() ) );
   }
}

