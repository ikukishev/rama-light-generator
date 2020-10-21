#pragma once

#include <QObject>
#include "qbassaudiofile.h"
#include "CConfiguration.h"

class CLightSequence : public QObject
{
   Q_OBJECT
public:
   explicit CLightSequence(const std::string& fileName, const CConfigation& configuration );

signals:

private:
    const CConfigation& m_configuration;
    std::shared_ptr<QBassAudioFile> m_audioFile;


};
