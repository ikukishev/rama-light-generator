#include <QVBoxLayout>
#include <QPushButton>
#include "ceffecteditorwidget.h"

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

   auto playPositionChanged = [ this ]( const SpectrumData& spectrum )
   {
      timeline->setCompositionPosition( spectrum.position );
   };

   m_spectrumConnections.push_back(
            std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( sequensePtr.get(), &CLightSequence::positionChanged, playPositionChanged ) ), deleter )
            );

   ;

   m_spectrumConnections.push_back(
            std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( timeline, &CTimeLineView::playFromPosition, [this](CTimeLineView* view, uint64_t position){
                                               auto sequensePtr = currentSequense.lock();
                                               if ( nullptr != sequensePtr )
                                               {
                                                  sequensePtr->getAudioFile()->setPosition( position );
                                               }
                                            } ) ), deleter )
            );

}
