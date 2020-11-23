#include "clightsequence.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QCheckBox>
#include <QProgressBar>
#include <QComboBox>
#include <QSlider>
#include <QFileInfo>
#include <QDebug>
#include <QJsonArray>
#include <QSizePolicy>
#include "csequensegenerator.h"


const QString cKeyFileName("file");
const QString cKeyChannelConfiguration("configuration");
const QString cKeyChannelUUID("uuid");
const QString cKeyChannelSpectrumIndex("spectrumIndex");
const QString cKeyChannelGain("gain");
const QString cKeyChannelMinimumLevel("minimumLevel");
const QString cKeyChannelFading("fading");
const QString cKeyChannelEffects("effects");


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
    m_audioFile = QBassAudioFile::get(m_fileName);
}


CLightSequence::~CLightSequence()
{
    destroy();
}

std::vector<QWidget *> CLightSequence::getControlWidgets()
{
   std::vector<QWidget*> controlWidgets;
   m_conncetionToDestroy.clear();
   if ( nullptr == m_audioFile )
   {
      qDebug() << "Was not able to create audio file";
      return controlWidgets;
   }

   //*************************************************************
   auto deleteButton = new QPushButton(  );
   deleteButton->setIcon( QIcon( ":/qss_icons/rc/window_close_focus.png" ) );
   connect(deleteButton, &QPushButton::clicked, [this]( bool/*checked*/){
      emit deleteTriggered( shared_from_this() );
   });

   auto upButton = new QPushButton( );
   upButton->setIcon( QIcon( ":/qss_icons/rc/arrow_up_focus.png" ) );
   connect(upButton, &QPushButton::clicked, [this](bool /*checked*/){
      emit moveUp( shared_from_this() );
   });

   auto downButton = new QPushButton( );
   downButton->setIcon( QIcon( ":/qss_icons/rc/arrow_down_focus.png" ) );
   connect(downButton, &QPushButton::clicked, [this](bool /*checked*/){
      emit moveDown( shared_from_this() );
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
   trackPosition->setValue( m_audioFile->position() );
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

   m_conncetionToDestroy.push_back( connectionPtr );

   //*************************************************************


   auto playButton = new QPushButton();
   playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPlay) );

   auto playButtonClickedEvent = [this]( bool ) {

       if ( isGenerateStarted() )
       {
           return;
       }

       if ( QBassAudioFile::EState::Play == m_audioFile->state() )
       {
          m_audioFile->stop();
       }
       else
       {
          m_audioFile->play();
       }
   };

   connect(playButton, &QPushButton::clicked, playButtonClickedEvent );

   m_conncetionToDestroy.push_back( std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( m_audioFile.get(), &QBassAudioFile::playStarted, [this, playButton](){
      playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPause) );
      if ( !m_isGenerateStarted )
      {
         sPlayEventDistributor.sendSequenseEvent( this );
         emit playStarted( shared_from_this() );
      }
   })), deleter ));


   m_conncetionToDestroy.push_back( std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( m_audioFile.get(), &QBassAudioFile::playStoped, [this, playButton](){
      playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPlay) );
      if ( !m_isGenerateStarted )
      {
         emit playStoped( shared_from_this() );
      }
   })), deleter ));


   m_conncetionToDestroy.push_back( std::shared_ptr<QMetaObject::Connection>(
               new QMetaObject::Connection( connect( m_audioFile.get(), &QBassAudioFile::playFinished, [this, playButton](){
      playButton->setIcon( playButton->style()->standardIcon(QStyle::SP_MediaPlay) );
      qDebug() << "emit emit playFinished( shared_from_this() );" ;
      if ( !m_isGenerateStarted )
      {
         emit playFinished( shared_from_this() );
      }
   })), deleter ));


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
   trackVolume->setValue( m_audioFile->getVolume() * 100 );

   connect(trackVolume, &QAbstractSlider::valueChanged, [this]( int value ){
         m_audioFile->setVolume( float(value)/100.0f );
   });

   //*************************************************************

   std::shared_ptr<QMetaObject::Connection> connectionFinishPtr( new QMetaObject::Connection(), deleter );

   m_conncetionToDestroy.push_back( connectionFinishPtr );

   auto genStop = [=]()
   {
      playButton->setDisabled(false);
      trackPosition->setDisabled(false);
      deleteButton->setDisabled(false);
      m_isGenerateStarted = false;
      disconnect(*connectionFinishPtr);
      m_audioFile->stop();
      trackVolume->setValue(100);
      m_audioFile->setVolume(1.0f);
   };

   auto getStart = [=]()
   {
      playButton->setDisabled(true);
      trackPosition->setDisabled(true);
      deleteButton->setDisabled(true);
      trackVolume->setValue(0);
      labelStatus->setText("Started");

      (*connectionFinishPtr) = connect( m_audioFile.get(), &QBassAudioFile::playFinished, [ this, genStop, labelStatus, trackVolume ](){
         if ( CSequenseGenerator::generateLms( this ) )
         {
             labelStatus->setText("Done");
         }
         else
         {
             labelStatus->setText("Done with error");
         }
         emit generationFinished( shared_from_this() );
         genStop();
      });

   };


   auto startButton = new QPushButton("Generate");
   startButton->setIcon(QIcon(":/images/record.png"));
   connect(startButton, &QPushButton::clicked, [ this, connectionFinishPtr, labelStatus, genStop, getStart ](bool){

      if ( m_isGenerateStarted )
      {
         genStop();
         labelStatus->setText("Stoped");
      }
      else
      {
         m_isGenerateStarted = true;
         m_audioFile->resetFFTData();
         m_audioFile->setVolume(0.0f);
         m_audioFile->play();
         emit generationStarted( shared_from_this() );
         getStart();
      }
   });

   if ( m_isGenerateStarted )
   {
      getStart();
   }

   //*************************************************************

   controlWidgets.clear();
   controlWidgets.reserve(10);

   controlWidgets.push_back( deleteButton );
   controlWidgets.push_back( upButton );
   controlWidgets.push_back( downButton );
   controlWidgets.push_back( label );
   controlWidgets.push_back( playButton );
   controlWidgets.push_back( startButton );
   controlWidgets.push_back( trackPosition );
   controlWidgets.push_back( durationLabel );
   controlWidgets.push_back( trackVolume );
   controlWidgets.push_back( labelStatus );

   return controlWidgets;
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
   m_audioFile = QBassAudioFile::get(m_fileName);
}


void CLightSequence::destroy()
{
   m_conncetionToDestroy.clear();
   if ( m_audioFile )
   {
      m_audioFile.reset();
      m_audioFile = nullptr;
   }
   qDebug() << __FUNCTION__ << m_fileName.c_str();
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

std::shared_ptr<CLightSequence::SequenceChannelConfigation> CLightSequence::getConfiguration(const QUuid &uuid) const
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

void CLightSequence::SequenceChannelConfigation::setGain(const double mul)
{
    if ( isGainSet() )
    {
        ( *gain ) = mul;
    }
    else
    {
        gain = std::make_shared< double >( mul );
    }
}

QJsonObject CLightSequence::SequenceChannelConfigation::serialize() const
{
    QJsonObject jo;
    jo[ cKeyChannelUUID ] = channelUuid.toString();
    if ( spectrumIndex ) jo[ cKeyChannelSpectrumIndex ] = static_cast<int>( *spectrumIndex );
    if ( gain ) jo[ cKeyChannelGain ] = static_cast<double>( *gain );
    jo[ cKeyChannelMinimumLevel ] = minimumLevel;
    jo[ cKeyChannelFading ] = fading;

    QJsonArray effectsJsonObjects;
    for ( auto& effect : effects )
    {
       if ( effect.second )
       {
          effectsJsonObjects.push_back( effect.second->toJson() );
       }
    }
    jo[ cKeyChannelEffects ] = effectsJsonObjects;

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

                if ( jo.contains( cKeyChannelGain ) )
                {
                    double m = jo[ cKeyChannelGain ].toDouble(-1.0);
                    if ( -1.0 != m )
                    {
                        cc->setGain(m);
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


                if ( jo.contains( cKeyChannelEffects ) )
                {
                   QJsonArray effectsArray = jo[ cKeyChannelEffects ].toArray();
                   for ( auto&& effect : effectsArray )
                   {
                      if ( effect.isObject() )
                      {
                         auto generator = IEffectGeneratorFactory::create(effect.toObject());
                         if ( generator )
                         {
                            cc->effects[ generator->getUuid() ] = generator;
                         }
                      }
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



void QLabelEx::mousePressEvent(QMouseEvent *event)
{
   emit clicked();
   QLabel::mousePressEvent(event);
}

void QSliderEx::mousePressEvent(QMouseEvent *event)
{
   emit clicked();
   QSlider::mousePressEvent(event);
}
