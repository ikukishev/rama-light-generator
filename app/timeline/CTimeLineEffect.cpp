#include "CTimeLineEffect.h"
#include <QDebug>
#include <QCursor>
#include <QGraphicsScene>

constexpr int64_t cDefaultDuration = 5000;

CTimeLineEffect::CTimeLineEffect( ITimeLineChannel* channel, uint64_t position, QObject* parent )
    : IEffect( parent )
    , oldPos( scenePos() )
{
    setFlags(ItemIsMovable);
    setEffectStartPosition( position );
    setEffectNameLabel( "base" );
    setParentItem( channel );
    setEffectDuration( cDefaultDuration );
    updatePosition();
}

QRectF CTimeLineEffect::boundingRect() const
{

   ITimeLineChannel* channel = getChannel();

   QRectF rect ( 0, 0, channel->timeLinePtr()->convertPositionToSceneX( effectDuration() ), channel->timeLinePtr()->channelHeight() );
   return rect;
}

void CTimeLineEffect::paint( QPainter *painter,
                            const QStyleOptionGraphicsItem *,
                            QWidget * )
{
    if ( auto channel = getChannel() )
    {
       updatePosition();

       painter->setBrush( channel->color() );
       QPen pen;
       pen.setWidth(1);
       if ( pressedForChangeDuration || pressedForMove )
       {
         pen.setColor( ~channel->color().rgb() );
       }
       painter->setPen(pen);

       auto rect = boundingRect();

       painter->drawRect( rect );
       QFont font = scene()->font();
       QFontMetricsF fontMetrics( font );
       auto fontRect = fontMetrics.boundingRect( effectNameLabel() );

       constexpr int padding = 3;

       if ( rect.width() > fontRect.width() + padding )
       {
          painter->drawText( padding, fontRect.height(), effectNameLabel() );
       }
    }
}


QVariant CTimeLineEffect::itemChange(GraphicsItemChange change, const QVariant &value)
{
   return QGraphicsItem::itemChange(change, value);
}

void CTimeLineEffect::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   oldMousePos = event->pos();
   oldPos = event->pos();

   if ( 7 > (boundingRect().width() - event->pos().x())  )
   {
      pressedForChangeDuration = true;
      pressedForMove = false;

      auto cur = this->cursor();
      cur.setShape( Qt::SizeHorCursor );
      this->setCursor(cur);
   }
   else
   {
      pressedForMove = true;
      pressedForChangeDuration = false;

      auto cur = this->cursor();
      cur.setShape( Qt::DragMoveCursor );
      this->setCursor(cur);
   }

   emit clicked( getChannel(), this );
   effectsSelected();
}


void CTimeLineEffect::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   QPointF newPos = event->pos();
    if ( pressedForMove )
    {
        ITimeLineChannel* channel = getChannel();

        int yDiff = newPos.y() - oldPos.y();

        int heightDiff= channel->timeLinePtr()->channelHeight() / 2;

        if (abs(yDiff) > heightDiff )
        {
           int offset = yDiff / heightDiff;
           auto newParent = channel->timeLinePtr()->getNeiborChannel( channel, offset );
           if ( nullptr != newParent )
           {
              parentItem()->update( parentItem()->boundingRect());
              setParentItem(newParent);
           }
        }

        int dx = newPos.x() - oldMousePos.x();
        auto realTrekPos = pos().x() - channel->timeLinePtr()->channelLabelWidth() + dx;
        if ( realTrekPos > 0 )
        {
           auto newPosition = channel->timeLinePtr()->convertSceneXToPosition( realTrekPos  );
           if ( newPosition + effectDuration() < channel->timeLinePtr()->compositionDuration() && newPosition > 0 )
           {
              setEffectStartPosition( newPosition );
           }
        }

        updatePosition();
        parentItem()->update( parentItem()->boundingRect());
    }
    else if ( pressedForChangeDuration )
    {

       ITimeLineChannel* channel = getChannel();

       int dx = newPos.x() - oldMousePos.x();

       auto posDx = channel->timeLinePtr()->convertSceneXToPosition( dx );

       auto newDuration = effectDuration()+posDx;
       setEffectDuration( newDuration );
       oldMousePos = newPos;
       parentItem()->update( parentItem()->boundingRect());
    }
}


void CTimeLineEffect::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{

   if ( oldPos != event->pos() )
   {
      effectChanged();
   }

   pressedForMove = false;
   pressedForChangeDuration = false;

   oldMousePos = event->pos();
   oldPos = event->pos();

   auto cur = this->cursor();
   cur.setShape( Qt::ArrowCursor );
   this->setCursor(cur);
}


void CTimeLineEffect::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Double Click";
    QGraphicsItem::mouseDoubleClickEvent(event);
}


CTimeLineEffect::EffectFactory CTimeLineEffect::EffectFactory::factory;

const QString &CTimeLineEffect::EffectFactory::menuLabel() const
{
   static QString label("test");
   return label;
}

IEffect *CTimeLineEffect::EffectFactory::create(ITimeLineChannel *parent, u_int64_t position)
{
   return new CTimeLineEffect( parent, position );
}
