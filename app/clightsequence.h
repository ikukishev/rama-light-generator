#pragma once

#include <QObject>
#include <QJsonObject>

#include "qbassaudiofile.h"
#include "CConfiguration.h"
#include "constants.h"
#include <memory>
#include <list>
#include <QSlider>
#include <QLabel>
#include "timeline/IEffectGenerator.h"


class CLightSequence;


class IInnerCommunicationGlue : public QObject
{
    Q_OBJECT

public:
    IInnerCommunicationGlue(QObject* parent);

    void sendSequenseEvent( CLightSequence* sequense );

signals:

   void sequenseEvent( CLightSequence* sequense );

};



class CLightSequence : public QObject
        , public std::enable_shared_from_this<CLightSequence>
{
   Q_OBJECT
public:

   class SequenceChannelConfigation
   {
       friend  class CLightSequence;
   public:
      SequenceChannelConfigation( const QUuid& achannelUuid ) : channelUuid( achannelUuid )
      {}

      SequenceChannelConfigation( const QUuid& achannelUuid, const uint32_t& spectrumIndex )
         : channelUuid( achannelUuid )
         , spectrumIndex( std::make_shared< uint32_t >( spectrumIndex ) )
      {}

      SequenceChannelConfigation( const QUuid& achannelUuid,
                                  const uint32_t& spectrumIndex,
                                  const double& again )
         : channelUuid( achannelUuid )
         , spectrumIndex( std::make_shared< uint32_t >( spectrumIndex ) )
         , gain( std::make_shared< double >( again ) )
      {}

      SequenceChannelConfigation( const QUuid& achannelUuid,
                                  const uint32_t& spectrumIndex,
                                  const double& again,
                                  const double& afade)
         : channelUuid( achannelUuid )
         , spectrumIndex( std::make_shared< uint32_t >( spectrumIndex ) )
         , gain( std::make_shared< double >( again ) )
         , fade( std::make_shared< double >( afade ) )
      {}


      bool isSpectrumIndexSet() const { return nullptr != spectrumIndex; }
      bool isGainSet() const { return nullptr != gain; }
      bool isFadeSet() const { return nullptr != fade; }
      bool isSameChannel( const QUuid& auuid ) const { return auuid == channelUuid; }

      void setSpectrumIndex( const uint32_t index );
      void setFade(const double fade );
      void setGain(const double fade );

      QJsonObject serialize() const;

      static std::shared_ptr<SequenceChannelConfigation> fromJson( const QJsonObject& jo );

   public:
      QUuid channelUuid;
      std::shared_ptr< uint32_t > spectrumIndex;
      std::shared_ptr< double > gain;
      double minimumLevel = cDefaultThreshholdValue;
      std::shared_ptr< double > fade;
      std::map< QUuid, std::shared_ptr<IEffectGenerator> > effects;
   };



   explicit CLightSequence(const std::string& fileName, const CConfigation& configuration);
   explicit CLightSequence(const std::string& fileName, const CConfigation& configuration,
                           std::list<std::shared_ptr<SequenceChannelConfigation>>&& channelConfiguration );
   ~CLightSequence();

   std::vector<QWidget *> getControlWidgets();

   bool isGenerateStarted() const { return m_isGenerateStarted; }
signals:
   void deleteTriggered( std::weak_ptr<CLightSequence> thisObject );
   void generationStarted( std::weak_ptr<CLightSequence> thisObject );
   void playStarted( std::weak_ptr<CLightSequence> thisObject );
   void playStoped( std::weak_ptr<CLightSequence> thisObject );
   void playFinished( std::weak_ptr<CLightSequence> thisObject );
   void moveUp( std::weak_ptr<CLightSequence> thisObject );
   void moveDown( std::weak_ptr<CLightSequence> thisObject );
   void generationFinished( std::weak_ptr<CLightSequence> thisObject );
   void positionChanged(const SpectrumData& spectrum);

private:

   void destroy();

public:
   void channelConfigurationUpdated();

   QJsonObject serialize() const;

   std::shared_ptr<SequenceChannelConfigation> getConfiguration( const QUuid& uuid ) const;

   const CConfigation& getGlobalConfiguration() const { return m_configuration; }

   static std::shared_ptr<CLightSequence> fromJson(const QJsonObject& jo, const CConfigation& configuration);

   const std::string& getFileName() const;

   const std::shared_ptr<QBassAudioFile>& getAudioFile() const { return m_audioFile; }

private:

   static IInnerCommunicationGlue sPlayEventDistributor;

private:
    const CConfigation& m_configuration;
    std::string m_fileName;
    std::shared_ptr<QBassAudioFile> m_audioFile;

    std::list<std::shared_ptr<SequenceChannelConfigation>> m_channelConfiguration;
    std::list<std::shared_ptr<QMetaObject::Connection>> m_conncetionToDestroy;

    bool m_isGenerateStarted;
};

