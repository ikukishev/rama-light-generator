#include "clightsequence.h"

CLightSequence::CLightSequence(const std::string &fileName, const CConfigation &configuration)
    : QObject(nullptr)
    , m_configuration(configuration)
    , m_audioFile( QBassAudioFile::get(fileName) )
{

}
