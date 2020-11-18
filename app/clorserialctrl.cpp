#include "clorserialctrl.h"
#include <QByteArray>
#include <QDebug>
#include <memory>
#include <algorithm>

const uint8_t    cHearbeatData[] = { 0x00, 0xFF, 0x81, 0x56, 0x00 };

CLORSerialCtrl::CLORSerialCtrl( QObject *parent )
    : QObject( parent )
    , m_serial( this )
    , m_timer( new QTimer( this ) )
{
    connect( m_timer, &QTimer::timeout, this, &CLORSerialCtrl::writeHeartbeatData );
}

CLORSerialCtrl::~CLORSerialCtrl()
{

}

bool CLORSerialCtrl::setPortParams(const QString &name, qint32 baudRate)
{
    if ( isOpen() )
    {
        if ( ( name == m_serial.portName() ) && ( m_serial.baudRate() == baudRate ) )
        {
            return true;
        }

        m_timer->stop();
        m_serial.close();
    }

    m_serial.setPortName(name);
    m_serial.setBaudRate( baudRate );

    if ( m_serial.open( QIODevice::WriteOnly ) )
    {
        m_timer->start( 500 );
        return true;
    }
    else
    {
        return false;
    }
}

bool CLORSerialCtrl::isOpen() const
{
    return m_serial.isOpen();
}

void CLORSerialCtrl::playStarted( std::weak_ptr<CLightSequence> currentSequense )
{
    m_sequenseConncetion.clear();
    auto sequense = currentSequense.lock();
    if ( nullptr != sequense )
    {
        auto deleter = [](QMetaObject::Connection* con){ disconnect(*con); delete con; };

        auto playPositionChanging = [ this
                , currentSequense
                , currentSpectrum = std::make_shared< SpectrumData >() ]
                (const SpectrumData& spectrum)
        {
            auto sequense = currentSequense.lock();
            if ( nullptr != sequense )
            {
                if ( currentSpectrum->position > spectrum.position || currentSpectrum->spectrum.size() != spectrum.spectrum.size() )
                {
                    *currentSpectrum = spectrum;
                }

                auto dt = spectrum.position - currentSpectrum->position;

                auto& channels = sequense->getGlobalConfiguration().channels();
                for ( auto& channel : channels )
                {
                    auto localConfiguration = sequense->getConfiguration( channel.uuid );

                    auto& effects = localConfiguration->effects;

                    bool useEffectValue = false;
                    double maxEffectValue = 0.0;

                    for ( auto& effect : effects )
                    {
                       if ( effect.second )
                       {
                          if ( effect.second->isPositionActive( spectrum.position ) )
                          {
                             auto effectValue = effect.second->generate( spectrum );
                             if ( effectValue > maxEffectValue )
                             {
                                maxEffectValue = effectValue;
                             }
                             useEffectValue = true;
                          }
                       }
                    }


                    auto spectrumIndex = channel.spectrumIndex;
                    if ( localConfiguration->isSpectrumIndexSet() )
                    {
                        spectrumIndex = *(localConfiguration->spectrumIndex);
                    }


                    auto k = (-1.0 / (1000.0 * ( localConfiguration->fading < 0.1 ? 0.1 : localConfiguration->fading )));
                    auto b = 1.0;
                    double y = k * double(dt) + b;
                    double intensityReduction = b - y;

                    currentSpectrum->spectrum[ spectrumIndex ] -= intensityReduction;


                    if ( !useEffectValue )
                    {
                       auto gain = channel.multipler;
                       if ( localConfiguration->isGainSet() )
                       {
                           gain = *(localConfiguration->gain);
                       }

                       auto value = spectrum.spectrum[ spectrumIndex ] * gain;


                       if ( value > currentSpectrum->spectrum[ spectrumIndex ] )
                       {
                           if ( value > 1.0 )
                           {
                               currentSpectrum->spectrum[ spectrumIndex ] = 1.0;
                           }
                           else if ( value > localConfiguration->minimumLevel )
                           {
                               currentSpectrum->spectrum[ spectrumIndex ] = value;
                           }
                       }
                    }
                    else
                    {
                       currentSpectrum->spectrum[ spectrumIndex ] = maxEffectValue;
                    }

                    //qDebug() << channel.label << "reduction:" << intensityReduction << "Intensity:" << currentSpectrum->spectrum[ spectrumIndex ];

                    setIntensity( channel, currentSpectrum->spectrum[ spectrumIndex ] );

                }

                currentSpectrum->position = spectrum.position;
            }
        };

        m_sequenseConncetion.push_back( { new QMetaObject::Connection( connect( sequense.get(), &CLightSequence::positionChanged, playPositionChanging )), deleter } );

    }
}

void CLORSerialCtrl::writeHeartbeatData()
{
    if ( isOpen() )
    {
        m_serial.write( reinterpret_cast<const char*>(cHearbeatData), sizeof ( cHearbeatData ) );
    }
}

void CLORSerialCtrl::setIntensity( const Channel& channel, double intensity )
{
    if ( isOpen() )
    {
        uint8_t inten = 0xf0;

        intensity = 1.0 - intensity;

        intensity *= 100.0;

        if ( intensity > 100.0 )
        {
            intensity = 100.0;
        }
        else if ( intensity < 0.0 )
        {
            intensity = 0.0;
        }

        intensity *= double(channel.voltage) / 220.0;

        if ( intensity > 99.0)
        {
            inten = 0xf0;
        }
        else if ( intensity < 1.0)
        {
            inten = 0x01;
        }
        else
        {
            intensity *= 2.0;
            intensity += 30.0;
            inten = static_cast< uint8_t >( intensity );
        }


        uint8_t channelByte = 0x80 | (0x0F & (channel.channel-1));
        auto unit = static_cast<uint8_t>(channel.unit);

        //qDebug() << "unit:" << channel.unit << "channel:" << channel.channel << "intensity:"<< intensity << "channelByte:" << channelByte;

        uint8_t  intensityData[6];
        intensityData[0] = 0x00;
        intensityData[1] = unit;
        intensityData[2] = 0x03;
        intensityData[3] = inten;
        intensityData[4] = channelByte;
        intensityData[5] = 0x00;

        m_serial.write( reinterpret_cast<const char*>(intensityData), sizeof ( intensityData ) );

    }
}
