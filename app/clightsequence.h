#pragma once

#include <QObject>
#include "qbassaudiofile.h"
#include "CConfiguration.h"

class CLightSequence : public QObject
{
   Q_OBJECT
public:
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

private:
    const CConfigation& m_configuration;
    std::string m_fileName;
    std::shared_ptr<QBassAudioFile> m_audioFile;

    std::vector<QWidget*> m_controlWidgets;
    //std::vector<std::shared_ptr<QMetaObject::Connection>> m_connections;

    bool m_isGenerateStarted;
};
