#include <QVBoxLayout>
#include <QPushButton>
#include "ceffecteditorwidget.h"
#include "timeline/CTimeLineEffect.h"

CEffectEditorWidget::CEffectEditorWidget(QWidget *parent)
   : QWidget(parent)
{
   auto layout = new QVBoxLayout( );

   timeline = new CTimeLineView( this );
   timeline->setChannelLabelWidth(200);

   timeline->setInteractive(true);
   timeline->setMouseTracking(true);
   timeline->setFocus();


   configurationArea = new QWidget( this );
   configurationArea->setMaximumHeight( 250 );
   configurationArea->setMinimumHeight( 250 );
   configurationAreaLayout = new QVBoxLayout( configurationArea );

   layout->addWidget( timeline );
   layout->addWidget( configurationArea );
   setLayout( layout );

}

void CEffectEditorWidget::setCurrentSequense(std::weak_ptr<CLightSequence> sequense)
{
   auto sequensePtr = sequense.lock();
   if ( nullptr == sequensePtr )
   {
      return;
   }

   m_spectrumConnections.clear();

   auto deleter = [](QMetaObject::Connection* con){ disconnect(*con); delete con; };

   currentSequense = sequense;

   timeline->setCompositionDuration( sequensePtr->getAudioFile()->duration() );

   timeline->clearChannels();
   if ( nullptr != configurationWidget )
   {
      delete configurationWidget;
      configurationWidget = nullptr;
   }


   const auto& channels = sequensePtr->getGlobalConfiguration().channels();

   for ( auto& channel : channels )
   {
      auto channelConfiguration = sequensePtr->getConfiguration( channel.uuid );
      if ( channelConfiguration )
      {
         auto timeLineChannel = new CTimeLineChannel( channel.uuid.toString(), channel.label, timeline );
         timeline->addChannel( timeLineChannel );
         timeLineChannel->setColor( channel.color );


         connect( timeLineChannel, &CTimeLineChannel::effectAdded, [ channelConfiguration ]( ITimeLineChannel*, IEffect* effect )
         {
            assert( nullptr != effect );
            if ( channelConfiguration->effects.end() == channelConfiguration->effects.find( effect->getUuid() ) )
            {
               channelConfiguration->effects.insert( { effect->getUuid(), effect->getEffectGenerator() } );
            }
         } );

         connect( timeLineChannel, &CTimeLineChannel::effectRemoved,  [ channelConfiguration, this ]( ITimeLineChannel* tlChannel, QUuid uuid )
         {

            qDebug() << "effectDeleted" <<  tlChannel->label() << uuid << "items";
            auto effectIt = channelConfiguration->effects.find( uuid );
            if ( effectIt != channelConfiguration->effects.end() )
            {
               channelConfiguration->effects.erase( effectIt );
            }
            if ( uuid == this->configurationWidgetEffectUuid )
            {
                if ( nullptr != configurationWidget )
                {
                   delete configurationWidget;
                   configurationWidget = nullptr;
                }
            }
         } );

         auto updateWidgetConfiguration = [this](  ITimeLineChannel*, IEffect* effect )
         {
            if ( nullptr != configurationWidget )
            {
               delete configurationWidget;
               configurationWidget = nullptr;
            }
            configurationWidget = effect->getEffectGenerator()->configurationWidget( configurationArea );
            configurationAreaLayout->addWidget( configurationWidget );
            configurationWidgetEffectUuid = effect->getUuid();
         };

         connect( timeLineChannel, &CTimeLineChannel::effectSelected, updateWidgetConfiguration );
         connect( timeLineChannel, &CTimeLineChannel::effectChanged, updateWidgetConfiguration );

         for ( auto& effect :  channelConfiguration->effects )
         {
            new CTimeLineEffect( timeLineChannel, effect.second );
         }
      }
   }


   auto playPositionChanged = [ this ]( const SpectrumData& spectrum )
   {
      timeline->setCompositionPosition( spectrum.position );
   };

   m_spectrumConnections.push_back(
            std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( sequensePtr.get(), &CLightSequence::positionChanged, playPositionChanged ) ), deleter )
            );

   m_spectrumConnections.push_back(
            std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( timeline, &CTimeLineView::playFromPosition, [this](CTimeLineView* , uint64_t position){
                                               auto sequensePtr = currentSequense.lock();
                                               if ( nullptr != sequensePtr )
                                               {
                                                  sequensePtr->getAudioFile()->setPosition( position );
                                                  if ( QBassAudioFile::EState::Play != sequensePtr->getAudioFile()->state() )
                                                  {
                                                     sequensePtr->getAudioFile()->play();
                                                  }
                                                  else
                                                  {
                                                     sequensePtr->getAudioFile()->stop();
                                                  }
                                               }
                                            } ) ), deleter )
            );

}
