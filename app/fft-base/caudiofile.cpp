#include "caudiofile.h"
#include <QDebug>
#include <QBuffer>



CAudioFile::CAudioFile(const QString& file_name , QObject *parent)
    : QObject( parent )
    , m_decoder( new QAudioDecoder(this) )
    , m_player( new QMediaPlayer(this) )
    , m_analyser( )
    , m_timer( new QTimer(this) )
{

    qRegisterMetaType<FrequencySpectrum>("FrequencySpectrum");
    qRegisterMetaType<WindowFunction>("WindowFunction");

    m_desiredFormat.setChannelCount(1);
    m_desiredFormat.setCodec("audio/pcm");
    m_desiredFormat.setSampleType(QAudioFormat::SignedInt);
    m_desiredFormat.setByteOrder(QAudioFormat::LittleEndian);
    m_desiredFormat.setSampleRate(48000);
    m_desiredFormat.setSampleSize(16);

    m_decoder->setAudioFormat( m_desiredFormat );
    m_decoder->setSourceFilename( file_name );

    m_player->setMedia(QUrl::fromLocalFile( file_name ));

    connect(m_decoder, SIGNAL(bufferReady()), this, SLOT(readBuffer()));
    connect(m_decoder, SIGNAL(finished()), this, SLOT(startFFTProcessing()));
    //connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(m_timer, &QTimer::timeout, [this]()
    {
        positionChanged(m_player->position());
    });
    m_analyser.setWindowFunction(HannWindow);

}




void CAudioFile::readBuffer()
{
    if ( m_decoder->bufferAvailable() )
    {
        auto audioData = m_decoder->read();
        m_audioData.append(audioData.constData<const char>(), audioData.byteCount());
        //qDebug() << __FUNCTION__ << " buffer size: " << m_audioData.size();
    }
}

void CAudioFile::startDecode()
{
    if ( !m_analyser.isReady() )
        m_analyser.cancelCalculation();
    m_decoder->stop();
    m_spectrumData.clear();
    m_decoder->start();
}


void CAudioFile::startFFTProcessing()
{
    static qint64 chunkSize = SpectrumLengthSamples*( m_desiredFormat.sampleSize() / 8 );

    qint64 startPosition = 0;

    if ( !m_spectrumData.empty() )
    {
        startPosition = m_spectrumData[m_spectrumData.size()-1]->startBytePosition + chunkSize;
    }


    if ( startPosition > (m_audioData.size()-chunkSize) )
    {
        qDebug() << __FUNCTION__ << " end!";
        emit processFinished();
    }
    else
    {

        std::shared_ptr<QMetaObject::Connection> connection = std::make_shared<QMetaObject::Connection>();
        *connection = connect( &m_analyser, &SpectrumAnalyser::spectrumChanged,
                               [this, startPosition, connection](const FrequencySpectrum &spectrum)
        {
            disconnect(*connection);
            auto position =  (qint64)(startPosition / (m_audioData.size() / (double)m_decoder->duration()));
            //qDebug() << __FUNCTION__ << " byte position: " << startPosition << ", position: " << position << ", duration: " << m_decoder->duration();
            m_spectrumData.push_back( std::make_shared<SpectrumAnalyserUnit>(startPosition, position, spectrum));
            startFFTProcessing();
        });

        QByteArray buffer( m_audioData.data() + startPosition, chunkSize );

        //qDebug() << __FUNCTION__ << " start: " << startPosition;
        m_analyser.calculate( buffer, m_desiredFormat );
    }
}

void CAudioFile::positionChanged(qint64 position)
{
    auto spectrumIndex = (std::size_t)(m_spectrumData.size() * (position / (double)duration()));
    if ( spectrumIndex > 0 && spectrumIndex < m_spectrumData.size() && !m_spectrumData.empty() )
    {
        qDebug() << __FUNCTION__ << " position: " << position << ", index: " << spectrumIndex << ",  calcPos: " << m_spectrumData[spectrumIndex]->position;
        emit positionChanged( position, m_spectrumData[spectrumIndex]->spectrum);
    }
}


void CAudioFile::play()
{
    if ( QMediaPlayer::PlayingState == m_player->state())
    {
        return;
    }

    m_player->play();
    m_timer->start(30);
}

void CAudioFile::stop()
{
    if ( QMediaPlayer::PlayingState == m_player->state())
    {
        m_player->pause();
    }
    m_timer->stop();
}

void CAudioFile::setPosition(qint64 position)
{
    if ( position < m_player->duration() && position > 0 )
    {
        m_player->setPosition(position);
    }
}
