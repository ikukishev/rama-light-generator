#pragma once

#include <QObject>
#include <QJsonObject>

#include "qbassaudiofile.h"
#include "CConfiguration.h"
#include <memory>
#include <list>


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
         , multipler( nullptr )
      {}

      SequenceChannelConfigation( const QUuid& achannelUuid,
                                  const uint32_t& spectrumIndex,
                                  const double& amultipler )
         : channelUuid( achannelUuid )
         , spectrumIndex( std::make_shared< uint32_t >( spectrumIndex ) )
         , multipler( std::make_shared< double >( amultipler ) )
      {}


      bool isSpectrumIndexSet() const { return nullptr != spectrumIndex; }
      bool isMultiplerSet() const { return nullptr != multipler; }
      bool isSameChannel( const QUuid& auuid ) const { return auuid == channelUuid; }

      void setSpectrumIndex( const uint32_t index );
      void setMultipler(const double mul );

      QJsonObject serialize() const;

      static std::shared_ptr<SequenceChannelConfigation> fromJson( const QJsonObject& jo );

   public:
      QUuid channelUuid;
      std::shared_ptr< uint32_t > spectrumIndex;
      std::shared_ptr< double > multipler;
      double minimumLevel = 0.0;
      double fading = 1.0;
   };



   explicit CLightSequence(const std::string& fileName, const CConfigation& configuration);
   explicit CLightSequence(const std::string& fileName, const CConfigation& configuration,
                           std::list<std::shared_ptr<SequenceChannelConfigation>>&& channelConfiguration );
   ~CLightSequence();

   const std::vector<QWidget*>& getControlWidgets() const { return m_controlWidgets; }

   bool isGenerateStarted() const { return m_isGenerateStarted; }
signals:
   void deleteTriggered( CLightSequence* thisObject );
   void generationStarted();
   void playStarted( CLightSequence* thisObject );
   void playFinished( CLightSequence* thisObject );
   void processFinished();
   void positionChanged(const SpectrumData& spectrum);

private:

   void adjust();
   void destroy();
   void generateSequense();


public:
   void channelConfigurationUpdated();

   QJsonObject serialize() const;

   std::shared_ptr<SequenceChannelConfigation> getConfiguration( const QUuid& uuid );

   static std::shared_ptr<CLightSequence> fromJson(const QJsonObject& jo, const CConfigation& configuration);

   const std::string& getFileName() const;

private:

   static IInnerCommunicationGlue sPlayEventDistributor;

private:
    const CConfigation& m_configuration;
    std::string m_fileName;
    std::shared_ptr<QBassAudioFile> m_audioFile;

    std::vector<QWidget*> m_controlWidgets;
    std::list<std::shared_ptr<SequenceChannelConfigation>> m_channelConfiguration;
    std::list<std::shared_ptr<QMetaObject::Connection>> m_conncetionToDestroy;

    bool m_isGenerateStarted;
};

