#include "ITimeLineTrackView.h"
#include <QDebug>

constexpr int64_t cMinimumDuration = 200;


ITimeLineChannel *IEffect::getChannel() const
{
   ITimeLineChannel* channel = dynamic_cast<ITimeLineChannel*>( parentItem() );
   assert( nullptr != channel );
   return channel;
}

void IEffect::updatePosition()
{
   ITimeLineChannel* channel = getChannel();
   setX( channel->timeLinePtr()->channelLabelWidth() + channel->timeLinePtr()->convertPositionToSceneX( effectStartPosition() ));
}

void IEffect::effectChanged()
{
   getChannel()->effectChangedEvent( this );
}

void IEffect::effectsSelected()
{
   getChannel()->effectSelectedEvent( this );
}

std::shared_ptr<IEffectGenerator> IEffect::getEffectGenerator() const
{
   return m_effectGenerator;
}


void IEffect::setEffectDuration(const int64_t &eD)
{
   if (getChannel()->timeLinePtr())
   {
      if ( eD > cMinimumDuration
           && ( getChannel()->timeLinePtr()->compositionDuration() > effectStartPosition() + eD ) )
      {
         m_effectGenerator->setEffectDuration( eD );
      }
      else if ( eD <= cMinimumDuration )
      {
         m_effectGenerator->setEffectDuration( cMinimumDuration );
      }
      else
      {
         m_effectGenerator->setEffectDuration( getChannel()->timeLinePtr()->compositionDuration() - effectStartPosition() - 1 );
      }
   }
   else
   {
      m_effectGenerator->setEffectDuration( cMinimumDuration );
   }
}

ITimeLineChannel::~ITimeLineChannel()
{
   for ( auto effect : effects() )
   {
      delete effect;
   }
}

std::list<IEffect *> ITimeLineChannel::effects() const
{
   std::list<IEffect *> effectList;
   for ( auto item : childItems() )
   {
      auto effect = dynamic_cast<IEffect*>( item );
      if ( nullptr != effect )
      {
         effectList.push_back( effect );
      }
   }
   return effectList;
}

void ITimeLineChannel::updateEffectPositions()
{
   for ( auto effect : effects() )
   {
      effect->updatePosition();
   }
}

void ITimeLineChannel::effectChangedEvent(IEffect *effect)
{
   emit effectChanged( this, effect );
}

void ITimeLineChannel::effectSelectedEvent(IEffect *effect)
{
   emit effectSelected( this, effect );
}

