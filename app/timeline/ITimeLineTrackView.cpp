#include "ITimeLineTrackView.h"
#include <QDebug>

constexpr int64_t cMinimumDuration = 200;

std::shared_ptr<std::list< IEffectFactory* >> IEffectFactory::sTrackFactories;

IEffectFactory::IEffectFactory()
{
   initTrackFactories().push_back( this );
}

const std::list<IEffectFactory *> &IEffectFactory::trackFactories()
{
   return initTrackFactories();
}

std::list<IEffectFactory *> &IEffectFactory::initTrackFactories()
{
   if ( nullptr == sTrackFactories )
   {
      sTrackFactories = std::make_shared<std::list<IEffectFactory *>>();
   }
   return *sTrackFactories;
}

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

const QUuid &IEffect::getUuid() const
{
   return m_uuid;
}

IEffect::IEffect(QObject *parent)
   : QObject( parent )
   , m_uuid( QUuid::createUuid() )
{}

void IEffect::setEffectDuration(const int64_t &eD)
{
   if (getChannel()->timeLinePtr())
   {
      if ( eD > cMinimumDuration
           && ( getChannel()->timeLinePtr()->compositionDuration() > effectStartPosition() + eD ) )
      {
         m_effectDuration = eD;
      }
      else if ( eD <= cMinimumDuration )
      {
         m_effectDuration = cMinimumDuration;
      }
      else
      {
         m_effectDuration = getChannel()->timeLinePtr()->compositionDuration() - effectStartPosition() - 1;
      }
   }
   else
   {
      m_effectDuration = cMinimumDuration;
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
