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
#include <QJsonArray>

const QString cKeyFileName("file");
const QString cKeyChannelConfiguration("configuration");
const QString cKeyChannelUUID("uuid");
const QString cKeyChannelSpectrumIndex("spectrumIndex");
const QString cKeyChannelMultipler("multipler");
const QString cKeyChannelMinimumLevel("minimumLevel");
const QString cKeyChannelFading("fading");


IInnerCommunicationGlue CLightSequence::sPlayEventDistributor(nullptr);


IInnerCommunicationGlue::IInnerCommunicationGlue(QObject *parent)
    : QObject(parent)
{}

void IInnerCommunicationGlue::sendSequenseEvent(CLightSequence *sequense)
{
    emit sequenseEvent( sequense );
}


CLightSequence::CLightSequence(const std::string &fileName, const CConfigation &configuration)
    : QObject( nullptr )
    , m_configuration( configuration )
    , m_fileName( fileName )
    , m_audioFile( nullptr )
    , m_isGenerateStarted( false )
{
    adjust();
}


CLightSequence::~CLightSequence()
{
    destroy();
}

CLightSequence::CLightSequence(const std::string &fileName,
                               const CConfigation &configuration,
                               std::list<std::shared_ptr<CLightSequence::SequenceChannelConfigation> > &&channelConfiguration)
    : QObject( nullptr )
    , m_configuration( configuration )
    , m_fileName( fileName )
    , m_audioFile( nullptr )
    , m_channelConfiguration( std::move( channelConfiguration ) )
    , m_isGenerateStarted( false )
{
    adjust();
}

void CLightSequence::adjust()
{
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

   auto playPositionChanging = [this, trackPosition, cacProgressLabel, durationLabel](const SpectrumData& spectrum){
      trackPosition->setValue(spectrum.position);
      durationLabel->setText( cacProgressLabel() );
      emit positionChanged( spectrum );
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

   auto playButtonClickedEvent = [this, playButton ](bool /*checked*/) {

       if ( isGenerateStarted() )
       {
           return;
       }

       if ( QBassAudioFile::EState::Play == m_audioFile->state() )
       {
          m_audioFile->stop();
          playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPlay) );
       }
       else if ( QBassAudioFile::EState::Stop == m_audioFile->state() )
       {
          m_audioFile->play();
          playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPause) );
          emit playStarted( this );
          sPlayEventDistributor.sendSequenseEvent( this );
       }

   };

   connect(playButton, &QPushButton::clicked, playButtonClickedEvent );

   connect( m_audioFile.get(), &QBassAudioFile::processFinished, [this, playButton](){
        playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPlay) );
        emit playFinished( this );
   });


   m_conncetionToDestroy.push_back( std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( &sPlayEventDistributor, &IInnerCommunicationGlue::sequenseEvent, [this, playButtonClickedEvent]( CLightSequence* sequense ){
       if ( this != sequense )
       {
           if (  QBassAudioFile::EState::Play == m_audioFile->state() )
           {
               playButtonClickedEvent( false );
           }
       }
   } )), deleter ));

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
         emit generationStarted();

         (*connectionFinishPtr) = connect( m_audioFile.get(), &QBassAudioFile::processFinished, [ this, genStop, labelStatus ](){
            generateSequense();
            emit processFinished();
            genStop();
            labelStatus->setText("Done");
         });
      }
   });

   //*************************************************************

   m_controlWidgets.clear();
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

   m_controlWidgets.clear();
   m_conncetionToDestroy.clear();
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
    QJsonObject jo;

    jo[ cKeyFileName ] = QString(m_fileName.c_str());

    QJsonArray configuration;

    for ( const auto& cc : m_channelConfiguration )
    {
        configuration.push_back( cc->serialize() );
    }

    jo[ cKeyChannelConfiguration ] = configuration;

    return jo;
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

std::shared_ptr<CLightSequence> CLightSequence::fromJson(const QJsonObject &jo, const CConfigation& configuration)
{
    std::shared_ptr<CLightSequence> ls;
    if ( jo.contains( cKeyFileName ) )
    {
        if ( !jo[ cKeyFileName ].isString() )
        {
            qWarning() << "filename of sequense is not a string";
        }
        else
        {
            QString fileName( jo[ cKeyFileName ].toString() );

            if ( QFile::exists( fileName ) )
            {
                std::list<std::shared_ptr<SequenceChannelConfigation>> channelConfiguration;
                if ( jo.contains( cKeyChannelConfiguration ) )
                {
                    for ( const auto& ccJo : jo[ cKeyChannelConfiguration ].toArray() )
                    {
                        if ( ccJo.isObject() )
                        {
                            auto ccPtr = SequenceChannelConfigation::fromJson( ccJo.toObject() );
                            if ( ccPtr )
                            {
                                channelConfiguration.push_back( ccPtr );
                            }
                        }
                    }
                }

                ls = std::make_shared<CLightSequence>( fileName.toStdString(), configuration, std::move( channelConfiguration ) );
            }
            else
            {
                qWarning() << "file is not exist";
            }
        }
    }

    return ls;
}

const std::string &CLightSequence::getFileName() const
{
    return m_fileName;
}

void CLightSequence::SequenceChannelConfigation::setSpectrumIndex(const uint32_t index)
{
    if ( isSpectrumIndexSet() )
    {
        ( *spectrumIndex ) = index;
    }
    else
    {
        spectrumIndex = std::make_shared< uint32_t >( index );
    }
}

void CLightSequence::SequenceChannelConfigation::setMultipler(const double mul)
{
    if ( isMultiplerSet() )
    {
        ( *multipler ) = mul;
    }
    else
    {
        multipler = std::make_shared< double >( mul );
    }
}

QJsonObject CLightSequence::SequenceChannelConfigation::serialize() const
{
    QJsonObject jo;
    jo[ cKeyChannelUUID ] = channelUuid.toString();
    if ( spectrumIndex ) jo[ cKeyChannelSpectrumIndex ] = static_cast<int>(*spectrumIndex);
    if ( multipler ) jo[ cKeyChannelMultipler ] = static_cast<double>(*multipler);
    jo[ cKeyChannelMinimumLevel ] = minimumLevel;
    jo[ cKeyChannelFading ] = fading;

    return jo;
}

std::shared_ptr<CLightSequence::SequenceChannelConfigation> CLightSequence::SequenceChannelConfigation::fromJson(const QJsonObject &jo)
{
    std::shared_ptr<SequenceChannelConfigation> cc;
    if ( jo.contains( cKeyChannelUUID ) )
    {
        if ( !jo[ cKeyChannelUUID ].isString() )
        {
            qWarning() << "Channel uuid not presentd";
        }
        else
        {
            QUuid uuid( jo[ cKeyChannelUUID ].toString() );
            if ( !uuid.isNull())
            {
                cc = std::make_shared<SequenceChannelConfigation>( uuid );
                if ( jo.contains( cKeyChannelSpectrumIndex ) )
                {
                    int index = jo[ cKeyChannelSpectrumIndex ].toInt(-1);
                    if ( -1 != index )
                    {
                        cc->setSpectrumIndex(index);
                    }
                }

                if ( jo.contains( cKeyChannelMultipler ) )
                {
                    double m = jo[ cKeyChannelMultipler ].toDouble(-1.0);
                    if ( -1.0 != m )
                    {
                        cc->setMultipler(m);
                    }
                }

                if ( jo.contains( cKeyChannelMinimumLevel ) )
                {
                    double m = jo[ cKeyChannelMinimumLevel ].toDouble(-1.0);
                    if ( -1.0 != m )
                    {
                        cc->minimumLevel = m;
                    }
                }

                if ( jo.contains( cKeyChannelFading ) )
                {
                    double m = jo[ cKeyChannelFading ].toDouble(-1.0);
                    if ( -1.0 != m )
                    {
                        cc->fading = m;
                    }
                }

            }
            else
            {
                qWarning() << "Channel uuid is wrong";
            }
        }
    }

    return cc;
}


