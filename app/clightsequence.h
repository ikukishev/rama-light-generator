#pragma once

#include <QObject>
#include <QJsonObject>

#include "qbassaudiofile.h"
#include "CConfiguration.h"
#include <memory>
#include <list>



class CLightSequence : public QObject
{
   Q_OBJECT
public:

   class SequenceChannelConfigation
   {
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


   public:
      QUuid channelUuid;
      std::shared_ptr< uint32_t > spectrumIndex;
      std::shared_ptr< double > multipler;
   };

   explicit CLightSequence(const std::string& fileName, const CConfigation& configuration );
   ~CLightSequence();

   const std::vector<QWidget*>& getControlWidgets() const { return m_controlWidgets; }

   bool isGenerateStarted() const { return m_isGenerateStarted; }
signals:
   void deleteTriggered(CLightSequence* thisObject);

private:
   void adjust();
   void destroy();
   void generateSequense();

public:
   void channelConfigurationUpdated();

   QJsonObject serialize() const;

   std::shared_ptr<SequenceChannelConfigation> getConfiguration( const QUuid& uuid );


private:
    const CConfigation& m_configuration;
    std::string m_fileName;
    std::shared_ptr<QBassAudioFile> m_audioFile;

    std::vector<QWidget*> m_controlWidgets;
    std::list<std::shared_ptr<SequenceChannelConfigation>> m_channelConfiguration;

    bool m_isGenerateStarted;
};
