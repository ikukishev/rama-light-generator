#ifndef QBASSAUDIOFILE_H
#define QBASSAUDIOFILE_H

#include <QObject>
#include <bass.h>
#include <memory>
#include <list>
#include <QTimer>
#include <QString>
#include "SpectrumData.h"

class QBassAudioFile: public QObject
{
    Q_OBJECT
private:
    QBassAudioFile();

public:

    enum class EState { Idle, Play, Stoped, Finished };

    ~QBassAudioFile();

    static std::shared_ptr<QBassAudioFile> get( const std::string& fileName );

    void play();
    void stop();
    void setPosition(uint64_t position);
    uint64_t position() const;
    uint64_t duration() const;

    void setFileName( const std::string& fileName );
    const std::string& fileName() const { return m_fileName; }

    const std::list< std::shared_ptr<SpectrumData> >& getSpectrum() const
    { return m_spectrumData; }

    void resetFFTData();

    float getVolume() const;
    void setVolume(const float vol ) const;

    const EState& state() const { return m_state; }


Q_SIGNALS:
    void playStarted();
    void playStoped();
    void playFinished();
    void positionChanged(const SpectrumData& spectrum);
    void playFinishedInternal();

private:
    static void CALLBACK bassStreamFinishedSyncProc(HSYNC, DWORD, DWORD, void *user );

    void playFinishedEvent();
private:

    std::string m_fileName;
    std::list< std::shared_ptr<SpectrumData> > m_spectrumData;
    QTimer * m_timer;
    HSTREAM m_stream;
    EState m_state;

};

#endif // QBASSAUDIOFILE_H
