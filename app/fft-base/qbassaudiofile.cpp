#include "qbassaudiofile.h"
#include <QDebug>
#include <QFile>


constexpr uint32_t cFFTSize = 128;

QBassAudioFile::QBassAudioFile()
    : m_fileName()
    , m_spectrumData()
    , m_timer( new QTimer(this) )
    , m_stream( 0 )
{
    connect( m_timer, &QTimer::timeout, [this]() {
        if ( 0 != m_stream )
        {
            auto pos = position();
            auto spectrum = std::make_shared<SpectrumData>( pos, std::vector<float>( cFFTSize ) );
            BASS_ChannelGetData( m_stream, spectrum->spectrum.data(), BASS_DATA_FFT256 );
            m_spectrumData.push_back(spectrum);

            emit positionChanged(*spectrum);
            if ( pos >= duration()-10)
            {
                stop();
                emit processFinished();
            }
        }
    });
}

std::shared_ptr<QBassAudioFile> QBassAudioFile::get( const std::string& fileName )
{
   static bool isInitialized = []() -> bool {
         if ( !BASS_Init(-1, 44100, 0, NULL, NULL) )
         {
             qDebug() << "Was not able to initialize BASS lirarry";
             return false;
         }
         return true;
   } ();

   std::shared_ptr<QBassAudioFile> bassAudio;
   if ( isInitialized )
   {
      bassAudio = std::shared_ptr<QBassAudioFile>( new QBassAudioFile() );
      bassAudio->setFileName(fileName);
   }

   return bassAudio;
}

void QBassAudioFile::play()
{
    if ( 0 != m_stream )
    {
        if ( !BASS_ChannelPlay( m_stream, false ) )
        {
            qDebug() << "BASS_ChannelPlay unsuccess" ;
        }
        m_timer->start(30);
    }
    else
    {
        qDebug() << "BASS_ChannelPlay unsuccess" ;
    }
}

void QBassAudioFile::stop()
{
    if ( 0 != m_stream )
    {
        BASS_ChannelStop( m_stream );
    }
    m_timer->stop();
}

void QBassAudioFile::setPosition(uint64_t position)
{
    if ( 0 != m_stream )
    {
        BASS_ChannelSetPosition( m_stream, BASS_ChannelSeconds2Bytes(m_stream, static_cast<double>(position)/1000.0), BASS_POS_BYTE );
    }
}

uint64_t QBassAudioFile::position() const
{
    if ( 0 != m_stream )
    {
        auto byte_pos = BASS_ChannelGetPosition(m_stream, BASS_POS_BYTE);
        return static_cast<uint64_t>(BASS_ChannelBytes2Seconds(m_stream, byte_pos)*1000);
    }
    else
    {
        return 0;
    }
}

uint64_t QBassAudioFile::duration() const
{
    if ( 0 != m_stream )
    {
        auto byte_pos = BASS_ChannelGetLength(m_stream, BASS_POS_BYTE);
        return static_cast<uint64_t>(BASS_ChannelBytes2Seconds(m_stream, byte_pos)*1000);
    }
    else
    {
        return 0;
    }
}

void QBassAudioFile::setFileName(const std::string &fileName)
{
    if ( !fileName.empty() )
    {
        if ( QFile::exists(fileName.c_str()) )
        {
            resetFFTData();
            m_fileName = fileName;

            if ( 0 != m_stream )
            {
                BASS_StreamFree(m_stream);
            }

            m_stream = BASS_StreamCreateFile(FALSE, fileName.c_str(), 0, 0, 0);
            if ( 0 == m_stream )
            {
                qDebug() << "Was not able to create stream from file" << fileName.c_str();
            }
        }
        else
        {
            qDebug() << "File not exist" << fileName.c_str();
        }
    }
    else
    {
        qDebug() << "filename empty: " << fileName.c_str();
    }
    qDebug() << "m_stream:" << m_stream;
}

void QBassAudioFile::resetFFTData()
{
    stop();
    m_spectrumData.clear();
    setPosition(0);
}
