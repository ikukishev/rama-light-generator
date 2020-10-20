#ifndef CAUDIOFILE_H
#define CAUDIOFILE_H


#include <QAudioDecoder>
#include <QString>
#include <QObject>
#include <QMediaPlayer>
#include <QTimer>
#include "spectrumanalyser.h"
#include <memory>



struct SpectrumAnalyserUnit
{
    SpectrumAnalyserUnit( qint64 bpos, qint64 apos, const FrequencySpectrum& aspectrum )
        : startBytePosition(bpos)
        , position(apos)
        , spectrum(aspectrum)
    {}
    qint64 startBytePosition = 0;
    qint64 position = 0;
    FrequencySpectrum spectrum;
};



class CAudioFile: public QObject
{
    Q_OBJECT
public:
    CAudioFile(const QString& file_name, QObject *parent = nullptr );

    void play();
    void stop();
    void setPosition(qint64 position);
    qint64 position() const { return m_player->position(); }
    qint64 duration() const { return m_player->duration(); }

    const std::vector< std::shared_ptr<SpectrumAnalyserUnit> >& getSpectrum() const
    { return m_spectrumData; }

public slots:
    void readBuffer();
    void startDecode();
    void startFFTProcessing();
    void positionChanged(qint64 position);

Q_SIGNALS:
    void processFinished();
    void positionChanged(qint64 position, const FrequencySpectrum& spectrum);

private:


private:
    QAudioDecoder *m_decoder;
    QMediaPlayer  *m_player;
    SpectrumAnalyser m_analyser;

    QByteArray m_audioData;
    std::vector< std::shared_ptr<SpectrumAnalyserUnit> > m_spectrumData;
    QAudioFormat m_desiredFormat;
    QTimer * m_timer;

};

#endif // CAUDIOFILE_H
