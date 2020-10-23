#include "clightsequence.h"
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QCheckBox>
#include <QProgressBar>
#include <QComboBox>
#include <QSlider>
#include <QFileInfo>
#include <QDebug>

CLightSequence::CLightSequence(const std::string &fileName, const CConfigation &configuration)
    : QObject(nullptr)
    , m_configuration(configuration)
    , m_fileName(fileName)
    , m_audioFile( nullptr )
    , m_isGenerateStarted( false )
{
   adjust();
}

CLightSequence::~CLightSequence()
{
   destroy();
}

void CLightSequence::adjust()
{
   if ( nullptr != m_audioFile )
   {
      destroy();
   }

   m_audioFile = QBassAudioFile::get(m_fileName);
   if ( nullptr == m_audioFile )
   {
      qDebug() << "Was not able to create audio file";
      return;
   }


   //*************************************************************

   auto deleteButton = new QPushButton("X");
   connect(deleteButton, &QPushButton::clicked, [this](bool /*checked*/){
      emit deleteTriggered(this);
   });

   //*************************************************************

   auto label = new QLabel(QFileInfo(m_fileName.c_str()).fileName());

   //*************************************************************

   auto labelStatus = new QLabel();

   //*************************************************************

   auto cacProgressLabel = [this]() -> QString {
         auto duration = m_audioFile->duration();
         int minutes = (duration/1000)/60;
         int seconds = (duration/1000)%60;
         auto position = m_audioFile->position();
         int posMinutes = (position/1000)/60;
         int posSeconds = (position/1000)%60;
         return "  " + QString::number(posMinutes) + ":" + QString::number(posSeconds) + " / " + QString::number(minutes) + ":" + QString::number(seconds);
   };

   auto durationLabel = new QLabel( cacProgressLabel() );


   //*************************************************************

   auto trackPosition = new QSlider( Qt::Horizontal );
   trackPosition->setMaximum( m_audioFile->duration() );
   trackPosition->setMinimum(0);

   auto playPositionChanging = [trackPosition, cacProgressLabel, durationLabel](const SpectrumData& spectrum){
      trackPosition->setValue(spectrum.position);
      durationLabel->setText( cacProgressLabel() );
   };

   auto deleter = [](QMetaObject::Connection* con){ disconnect(*con); delete con; };

   std::shared_ptr<QMetaObject::Connection> connectionPtr( new QMetaObject::Connection( connect( m_audioFile.get(), &QBassAudioFile::positionChanged, playPositionChanging )), deleter );

   connect(trackPosition, &QAbstractSlider::sliderPressed, [connectionPtr](){
      disconnect( *connectionPtr );
   });

   connect(trackPosition, &QAbstractSlider::sliderReleased, [this, trackPosition, connectionPtr, playPositionChanging ](){
      (*connectionPtr) = connect( m_audioFile.get(), &QBassAudioFile::positionChanged, playPositionChanging );
      m_audioFile->setPosition( trackPosition->value() );
   });

   //*************************************************************


   auto playButton = new QPushButton();
   playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPlay) );
   connect(playButton, &QPushButton::clicked, [this, playButton ](bool /*checked*/){
      if ( QBassAudioFile::EState::Play == m_audioFile->state() )
      {
         m_audioFile->stop();
         playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPlay) );
      }
      else if ( QBassAudioFile::EState::Stop == m_audioFile->state() )
      {
         m_audioFile->play();
         playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPause) );
      }
   });

   //*************************************************************

   auto trackVolume = new QSlider( Qt::Horizontal );
   trackVolume->setMaximum( 100 );
   trackVolume->setMinimum( 0 );
   trackVolume->setValue( 100 );

   connect(trackVolume, &QAbstractSlider::valueChanged, [this]( int value ){
         m_audioFile->setVolume( float(value)/100.0f );
   });

   //*************************************************************

   std::shared_ptr<QMetaObject::Connection> connectionFinishPtr( new QMetaObject::Connection(), deleter );

   auto startButton = new QPushButton("Generate");
   startButton->setIcon(QIcon(":/images/record.png"));
   connect(startButton, &QPushButton::clicked, [ this, playButton, trackPosition,
           deleteButton, connectionFinishPtr, labelStatus, trackVolume ](bool /*checked*/){

      auto genStop = [=]()
      {
         playButton->setDisabled(false);
         trackPosition->setDisabled(false);
         deleteButton->setDisabled(false);
         m_isGenerateStarted = false;
         disconnect(*connectionFinishPtr);
      };

      if ( m_isGenerateStarted )
      {
         genStop();
         labelStatus->setText("Stoped");
      }
      else
      {
         playButton->setDisabled(true);
         trackPosition->setDisabled(true);
         deleteButton->setDisabled(true);
         m_isGenerateStarted = true;
         m_audioFile->resetFFTData();
         m_audioFile->setVolume(0.0f);
         trackVolume->setValue(0);
         m_audioFile->play();
         labelStatus->setText("Started");

         (*connectionFinishPtr) = connect( m_audioFile.get(), &QBassAudioFile::processFinished, [ this, genStop, labelStatus ](){
            generateSequense();
            genStop();
            labelStatus->setText("Done");
         });
      }
   });

   //*************************************************************

   m_controlWidgets.reserve(8);

   m_controlWidgets.push_back( deleteButton );
   m_controlWidgets.push_back( label );
   m_controlWidgets.push_back( playButton );
   m_controlWidgets.push_back( startButton );
   m_controlWidgets.push_back( trackPosition );
   m_controlWidgets.push_back( durationLabel );
   m_controlWidgets.push_back( trackVolume );
   m_controlWidgets.push_back( labelStatus );

}

void CLightSequence::destroy()
{
   if ( m_audioFile )
   {
      m_audioFile->resetFFTData();
      m_audioFile.reset();
      m_audioFile = nullptr;
   }

   for ( auto widget : m_controlWidgets )
   {
      if ( nullptr != widget )
      {
         delete widget;
      }
   }
   m_controlWidgets.clear();
   qDebug() << __FUNCTION__ << m_fileName.c_str();
}

void CLightSequence::generateSequense()
{
   qDebug() << __FUNCTION__;
}

void CLightSequence::channelConfigurationUpdated()
{
   std::list<decltype (m_channelConfiguration.begin())> forCleanup;
   for ( auto cc = m_channelConfiguration.begin(); cc != m_channelConfiguration.end(); ++cc )
   {
      if ( !m_configuration.hasChannel( (*cc)->channelUuid ) )
      {
         forCleanup.push_back( cc );
      }
   }

   for ( auto it : forCleanup )
   {
      m_channelConfiguration.erase( it );
   }

   for( const auto& channel : m_configuration.channels() )
   {
      if ( nullptr == getConfiguration( channel.uuid ) )
      {
         m_channelConfiguration.push_back( std::make_shared<SequenceChannelConfigation>( channel.uuid ));
      }
   }
}

QJsonObject CLightSequence::serialize() const
{
   return QJsonObject();
}

std::shared_ptr<CLightSequence::SequenceChannelConfigation> CLightSequence::getConfiguration(const QUuid &uuid)
{
   std::shared_ptr<SequenceChannelConfigation> cc(nullptr);

   for ( auto& ptr : m_channelConfiguration )
   {
      if ( uuid == ptr->channelUuid )
      {
         cc = ptr;
         break;
      }
   }

   return cc;
}
