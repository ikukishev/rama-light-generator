#include "clorserialctrl.h"
#include <QByteArray>
#include <QDebug>
#include <memory>
#include <algorithm>

const uint8_t    cHearbeatData[] = { 0x00, 0xFF, 0x81, 0x56, 0x00 };
constexpr auto   cChannelsPerUnit = 32;

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

    m_timer->start( 500 );

    return m_serial.open( QIODevice::WriteOnly );

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
                , currentIntensity = std::map< int /*unit*32+channel*/, float /*CurrentIntensity*/>()
                , currentPosition = uint64_t(0) ]
                (const SpectrumData& spectrum) mutable
        {
            auto sequense = currentSequense.lock();
            if ( nullptr != sequense )
            {
                auto dt = spectrum.position - currentPosition;

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

                    auto fade = channel.fade;
                    if ( localConfiguration->isFadeSet() )
                    {
                        fade = *(localConfiguration->fade);
                    }


                    auto k = (-1.0 / (1000.0 * ( fade < 0.1 ? 0.1 : fade )));
                    auto b = 1.0;
                    double y = k * double(dt) + b;
                    double intensityReduction = b - y;

                    int channelIndex = channel.unit * cChannelsPerUnit + channel.channel;
                    auto currentChannelIntensityIt = currentIntensity.find(channelIndex);
                    if ( currentChannelIntensityIt == currentIntensity.end() )
                    {
                        currentChannelIntensityIt = currentIntensity.insert( {channelIndex, 0.0f } ).first;
                    }
                    else if ((*currentChannelIntensityIt).second > 0.0f)
                    {
                        (*currentChannelIntensityIt).second -= intensityReduction;
                    }


                    if ( !useEffectValue )
                    {
                       auto gain = channel.gain;
                       if ( localConfiguration->isGainSet() )
                       {
                           gain = *(localConfiguration->gain);
                       }

                       auto value = spectrum.spectrum[ spectrumIndex ] * gain;


                       if ( value > (*currentChannelIntensityIt).second )
                       {
                           if ( value > 1.0 )
                           {
                               (*currentChannelIntensityIt).second = 1.0;
                           }
                           else if ( value > localConfiguration->minimumLevel )
                           {
                               (*currentChannelIntensityIt).second = value;
                           }
                       }
//                     if ( channel.unit == 2 && channel.channel == 8 )
                       {
                          qDebug() << "unit:" << channel.unit << "voltage:" << channel.voltage << "channel:" << channel.channel << "fade:" << fade << "gain:" << gain << "spectrumIndex:" << spectrumIndex << "Intensity:" << (*currentChannelIntensityIt).second;
                       }
                    }
                    else
                    {
                       (*currentChannelIntensityIt).second = maxEffectValue;
                    }

                    //qDebug() << channel.label << "reduction:" << intensityReduction << "Intensity:" << currentSpectrum->spectrum[ spectrumIndex ];

                    setIntensity( channel, (*currentChannelIntensityIt).second );

                }

                currentPosition = spectrum.position;
            }
        };

        m_sequenseConncetion.push_back( { new QMetaObject::Connection( connect( sequense.get(), &CLightSequence::positionChanged, playPositionChanging )), deleter } );

    }
}

void CLORSerialCtrl::writeHeartbeatData()
{
    if ( QSerialPort::NoError != m_serial.error() )
    {
        qDebug() << "Serial port error:" << m_serial.error() << m_serial.errorString();
        if ( isOpen() )
        {
            m_serial.close();
        }
    }

    if ( !isOpen() )
    {
        m_serial.open( QIODevice::WriteOnly );
    }


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

        intensity *= double(channel.voltage) / 220.0;
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

        //qDebug() << "unit:" << channel.unit << "voltage:" << channel.voltage << "channel:" << channel.channel << "intensity:"<< intensity;

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
