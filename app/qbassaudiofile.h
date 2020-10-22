#ifndef QBASSAUDIOFILE_H
#define QBASSAUDIOFILE_H

#include <QObject>
#include <bass.h>
#include <vector>
#include <memory>
#include <QTimer>
#include <QString>

struct SpectrumData
{
    SpectrumData( uint64_t apos, const std::vector<float>&& aspectrum )
        : position( apos )
        , spectrum( std::move(aspectrum) )
    {}
    SpectrumData() = default;
    uint64_t position = 0;
    std::vector<float> spectrum;
};


class QBassAudioFile: public QObject
{
    Q_OBJECT
private:
    QBassAudioFile();

public:

    enum class EState { Play, Stop };

    ~QBassAudioFile();

    static std::shared_ptr<QBassAudioFile> get( const std::string& fileName );

    void play();
    void stop();
    void setPosition(uint64_t position);
    uint64_t position() const;
    uint64_t duration() const;

    void setFileName( const std::string& fileName );
    const std::string& fileName() const { return m_fileName; }

    const std::vector< std::shared_ptr<SpectrumData> >& getSpectrum() const
    { return m_spectrumData; }

    void resetFFTData();

    float getVolume() const;
    void setVolume(const float vol ) const;

    const EState& state() const { return m_state; }

Q_SIGNALS:
    void processFinished();
    void positionChanged(const SpectrumData& spectrum);

private:
    std::string m_fileName;
    std::vector< std::shared_ptr<SpectrumData> > m_spectrumData;
    QTimer * m_timer;
    HSTREAM m_stream;
    EState m_state;

};

#endif // QBASSAUDIOFILE_H
