#include <QGraphicsScene>
#include <QMouseEvent>
#include <QMenu>

#include <QDebug>
#include <QAction>

#include "CTimeLineView.h"
#include "CTimeLineEffect.h"

CTimeLineView::CTimeLineView( QWidget *parent )
    : QGraphicsView( new QGraphicsScene( ), parent )
    , ITimeLineTrackView()
{

    setMouseTracking( true );

    indicator = new CTimeLineIndicator( );
    indicator->setTimeLinePtr( this );

    indicator->setZValue( 102 );

    scene()->addItem( indicator );


    indicatorPosition = new CTimeLinePosition();

    indicatorPosition ->setTimeLinePtr( this );

    indicatorPosition ->setZValue( 101 );

    scene()->addItem( indicatorPosition );

    setBackgroundBrush( QBrush( Qt::black ) );

}

void CTimeLineView::mousePressEvent(QMouseEvent *event)
{
   if ( Qt::RightButton == event->button() )
   {
      auto item = itemAt( event->pos() );
      if ( nullptr != item )
      {
         if ( auto channel = dynamic_cast< CTimeLineChannel* >(item) )
         {
             qDebug() << event->pos() << item << "channel:" << channel->label();
             QMenu menu(this);
             for ( auto f : IEffectFactory::trackFactories() )
             {
                 QAction* newAct = new QAction( f->menuLabel(), this );
                 menu.addAction( newAct );
                 connect( newAct, &QAction::triggered, [ this, f, channel, pos = mapToScene(event->pos()) ]()
                 {
                    f->create( channel, convertSceneXToPosition( pos.x() ) );
                 });
             }
             menu.exec( QCursor::pos() );
         }
         else if ( auto track = dynamic_cast< IEffect* >(item) )
         {
            qDebug() << event->pos() << "track:" << track->effectNameLabel() << "position:" << track->effectStartPosition();
            QMenu menu( this );
            QAction* newAct = new QAction( "Remove \"" + track->effectNameLabel() + "\" from " + track->getChannel()->label(), this );
            menu.addAction( newAct );
            connect( newAct, &QAction::triggered, [ track ]()
            {
               track->setParentItem( nullptr );
               delete track;
            });
            menu.exec( QCursor::pos() );
         }
      }
   }

   QGraphicsView::mousePressEvent( event );
}

void CTimeLineView::mouseMoveEvent( QMouseEvent *event )
{
    QGraphicsView::mouseMoveEvent(event);
}

void CTimeLineView::AddItem(QPointF pos, QRect rect, QPen pen, QBrush brush)
{
    QGraphicsItem *item = scene()->addRect(rect,pen,brush);
    item->setPos(pos);
}

void CTimeLineView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void CTimeLineView::keyPressEvent(QKeyEvent *event)
{

   if ( Qt::Key_Alt == event->key() )
   {
      m_isAltPressed = true;
   }
   else if ( m_isAltPressed && Qt::Key_0 == event->key() )
   {
      m_scale = 1.0f;
      updateSceneRect();
   }
   else if ( Qt::Key_Space == event->key() )
   {
      emit playFromPosition(this, indicator->position() );
   }

   QGraphicsView::keyPressEvent(event);
}

void CTimeLineView::resizeEvent(QResizeEvent *event)
{
   updateSceneRect();
   QGraphicsView::resizeEvent(event);
}

void CTimeLineView::keyReleaseEvent(QKeyEvent *event)
{
   if ( Qt::Key_Alt == event->key() )
   {
      m_isAltPressed = false;
   }
   QGraphicsView::keyReleaseEvent( event );
}

void CTimeLineView::wheelEvent(QWheelEvent *event)
{
   if ( m_isAltPressed )
   {
      auto steps = event->angleDelta().x() / 8.0f;
      setScale( m_scale + ( steps / 720.f ) );
      updateSceneRect();
   }
   update();

   QGraphicsView::wheelEvent( event );
}

void CTimeLineView::drawBackground(QPainter *painter, const QRectF &rect)
{
   QGraphicsView::drawBackground(painter, rect);

   QPen pen;
   pen.setWidth( 1 );
   pen.setColor( Qt::darkRed );
   painter->setPen( pen );
   painter->setBrush( QColor(10, 10, 10 ) );
   painter->drawRect( 0, 0, scene()->width() - m_channelLabelWidth - cFieldMargin,
                      scene()->height() - m_timeLabelsHeight - cFieldMargin );

}

void CTimeLineView::updateSceneRect()
{
   int w = ( width() - m_channelLabelWidth ) * m_scale + m_channelLabelWidth;
   int h = m_channels.size() * m_channelHeight + m_timeLabelsHeight;

   if ( height() > h )
   {
      h = height();
   }

   scene()->setSceneRect( -int( m_channelLabelWidth ), -int( m_timeLabelsHeight ), w - cFieldMargin, h - cFieldMargin);

   for ( int i = 0; i < int(m_channels.size()); ++i )
   {
      m_channels[i]->setPos( -int(m_channelLabelWidth), i * m_channelHeight );
      m_channels[i]->updateChannelGraphics();
   }

   indicator->updateScenePosition();

}

uint32_t CTimeLineView::channelHeight() const
{
   return m_channelHeight;
}


int64_t CTimeLineView::compositionDuration() const
{
   return m_compositionDuration;
}

uint64_t CTimeLineView::channelLabelWidth() const
{
   return m_channelLabelWidth;
}

int64_t CTimeLineView::compositionPosition() const
{
   return m_compositionPosition;
}

qreal CTimeLineView::convertPositionToSceneX() const
{
   return convertPositionToSceneX( m_compositionPosition );
}

qreal CTimeLineView::convertPositionToSceneX(int64_t position) const
{
   if ( nullptr != scene() )
   {
      double fieldWidth = scene()->width() - m_channelLabelWidth - cFieldMargin;
      double factor = double(position) / double(m_compositionDuration);
      return factor * fieldWidth;
   }
   return 0;
}

int64_t CTimeLineView::convertSceneXToPosition(qreal x) const
{
   double fieldWidth = scene()->width() - m_channelLabelWidth - cFieldMargin;
   double factor = x / fieldWidth;
   return int64_t( factor * m_compositionDuration );
}

ITimeLineChannel *CTimeLineView::getNeiborChannel(ITimeLineChannel *channel, int offsetIndex) const
{
   ITimeLineChannel *res = nullptr;

   int index = -1;
   for ( std::size_t i = 0; i < m_channels.size(); ++i )
   {
      if ( m_channels[i] == channel )
      {
         index = i;
         break;
      }
   }

   if ( -1 != index )
   {
      index += offsetIndex;
      if ( index < 0 )
      {
         index = 0;
      }
      else if ( index >= int(m_channels.size()) )
      {
         index = int(m_channels.size()) - 1;
      }
      res = m_channels[ index ];
   }

   return res;
}

void CTimeLineView::addChannel( CTimeLineChannel *channel )
{
   if ( nullptr == channel)
   {
      return;
   }

   m_channels.push_back( channel );
   channel->setTimeLinePtr( this );
   scene()->addItem( channel );
}

void CTimeLineView::removeChannel(CTimeLineChannel *channel)
{
   if ( nullptr == channel)
   {
      return;
   }

   auto it = std::find( m_channels.begin(), m_channels.end(), channel );
   if ( m_channels.end() != it )
   {
      if (*it)
      {
         delete *it;
         m_channels.erase( it);
         updateSceneRect();
      }
   }
}

void CTimeLineView::clearChannels()
{
   for ( auto ch : m_channels )
   {
      if ( ch )
      {
         delete ch;
      }
   }
   m_channels.clear();
   updateSceneRect();
}

void CTimeLineView::setChannelLabelWidth(const uint32_t &channelLabelWidth)
{
   m_channelLabelWidth = channelLabelWidth;
   updateSceneRect();
}

uint32_t CTimeLineView::timeLabelsHeight() const
{
   return m_timeLabelsHeight;
}

void CTimeLineView::setTimeLabelsHeight( const uint32_t &tlh )
{
   m_timeLabelsHeight = tlh < 50 ? 50 : tlh;
}

float CTimeLineView::scale() const
{
   return m_scale;
}

void CTimeLineView::setScale(float scale)
{
    m_scale = scale < 1.0f ? 1.0f : scale;
}

const std::vector< CTimeLineChannel* >& CTimeLineView::channels() const
{
   return m_channels;
}

void CTimeLineView::setChannelHeight(uint32_t &&ch)
{
   m_channelHeight =  ch < 20 ? 20 : ch ;
}

void CTimeLineView::setCompositionDuration( int64_t length )
{
   if ( length > 0 )
   {
      m_compositionDuration = length;
      setCompositionPosition( 0 );
      indicator->setPosition( 0 );
      update();
   }
}

void CTimeLineView::setCompositionPosition( int64_t position )
{
   if ( position >=0 && position < m_compositionDuration )
   {
      m_compositionPosition = position;
      indicatorPosition->setPosition( position );
      update();
   }
}
