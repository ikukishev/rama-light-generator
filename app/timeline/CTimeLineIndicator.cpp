#include <QDebug>
#include <QGraphicsScene>
#include "CTimeLineIndicator.h"

constexpr qreal cIndicatorSize = 7;

CTimeLineIndicator::CTimeLineIndicator()
   : QGraphicsItem ()
   , brush( Qt::RoundCap )
   , pen( Qt::darkMagenta,1 )
{
    brush.setColor(QColor("#50f"));
    points<<QPointF(-cIndicatorSize,0)
         <<QPointF(0,cIndicatorSize)
        <<QPointF(cIndicatorSize,0)
       <<QPointF(cIndicatorSize,-cIndicatorSize)
      <<QPointF(-cIndicatorSize,-cIndicatorSize);

   setAcceptHoverEvents(true);
   this->setAcceptDrops(true);

   setFlag(QGraphicsItem::ItemIsMovable);
   setFlag(QGraphicsItem::ItemIsFocusable);
   setFlag(ItemSendsGeometryChanges);
}

QSizeF CTimeLineIndicator::calculateSize()const
{
    float minX = points[0].x();
    float minY = points[0].y();
    float maxX = points[0].x();
    float maxY = points[0].y();
    for(QPointF point : points){
        if (point.x() < minX){
            minX = point.x();
        }
        if (point.y() < minY){
            minY = point.y();
        }
        if (point.x() > maxX){
            maxX = point.x();
        }
        if (point.y() > maxY){
            maxY = point.y();
        }
    }
    return QSizeF(maxX-minX,line.p2().y());
}

int64_t CTimeLineIndicator::position() const
{
   return m_position;
}

ITimeLineTrackView *CTimeLineIndicator::TimeLinePtr() const
{
   return m_TimeLinePtr;
}

void CTimeLineIndicator::setTimeLinePtr(ITimeLineTrackView *TimeLinePtr)
{
   m_TimeLinePtr = TimeLinePtr;
}

void CTimeLineIndicator::setPosition(int64_t position)
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

void CTimeLineIndicator::updateScenePosition()
{
   if ( nullptr != m_TimeLinePtr )
   {

      setX( m_TimeLinePtr->convertPositionToSceneX( m_position ) );
   }
}

QRectF CTimeLineIndicator::boundingRect() const
{
    QSizeF size = this->calculateSize();
    return QRectF(-5,-5,size.width(),size.height());
}

void CTimeLineIndicator::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
   if ( scene() )
   {
      setHeight( scene()->height() );
   }

   if ( !pressed )
   {
      updateScenePosition();
   }

   painter->setPen(pen);
   painter->drawLine(line);
   painter->setBrush(brush);
   painter->drawPolygon(points);
}

void CTimeLineIndicator::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void CTimeLineIndicator::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->scenePos();
    if ( pressed )
    {
       if ( m_TimeLinePtr )
       {
          setPosition( m_TimeLinePtr->convertSceneXToPosition( pos.x() ) );
       }
       this->setPos( pos.x(), y() );
    }
    QGraphicsItem::mouseMoveEvent(event);
    update();
}

void CTimeLineIndicator::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    qDebug()<<event->scenePos();
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

QVariant CTimeLineIndicator::itemChange(GraphicsItemChange change, const QVariant &value)
{

    if ( change == ItemPositionChange && scene() )
    {
        // value is the new position.
        QPointF newPos = value.toPointF();
        newPos.setY(y());
        if( newPos.x() < 0 || nullptr == m_TimeLinePtr )
        {
            newPos.setX(0);
        }
        else
        {
           auto rightPosLimit = scene()->width() - m_TimeLinePtr->channelLabelWidth() - cFieldMargin;
           if ( newPos.x() > rightPosLimit )
           {
              newPos.setX( rightPosLimit );
           }
        }
        return newPos;
    }
    return QGraphicsItem::itemChange(change, value);
}
