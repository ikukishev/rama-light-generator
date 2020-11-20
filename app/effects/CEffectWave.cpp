
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <cmath>

#include "CEffectWave.h"

const QString cKeyPhaseShift ( "phaseShift" );
const QString cKeyWaveLength( "waveLength" );
const QString cKeyWaveGain ( "waveGain" );
const QString cKeyWaveAmplitudeShift( "waveAmplitudeShift" );


QJsonObject CEffectWave::toJsonParameters() const
{
   QJsonObject parameters;

   parameters[ cKeyPhaseShift ] = m_phaseShift;
   parameters[ cKeyWaveLength ] = m_waveLength;
   parameters[ cKeyWaveGain ] = m_waveGain;
   parameters[ cKeyWaveAmplitudeShift ] = m_waveAmplitudeShift;

   return parameters;
}

bool CEffectWave::parseParameters(const QJsonObject &parameters)
{
   bool isOk = false;
   if ( parameters.contains( cKeyPhaseShift )
        && parameters.contains( cKeyWaveLength )
        && parameters.contains( cKeyWaveGain )
        && parameters.contains( cKeyWaveAmplitudeShift )  )
   {
      m_phaseShift = parameters[ cKeyPhaseShift ].toDouble( 0.0 );
      m_waveLength = parameters[ cKeyWaveLength ].toDouble( 1.0 );
      m_waveGain = parameters[ cKeyWaveGain ].toDouble( 1.0 );
      m_waveAmplitudeShift = parameters[ cKeyWaveAmplitudeShift ].toDouble( 0.0 );
      isOk = true;
   }
   return isOk;
}

double CEffectWave::calculateIntensity(const SpectrumData &spectrumData)
{
   auto dtMs = spectrumData.position - effectStartPosition();
   if ( dtMs < 0 )
      dtMs = 0;
   double dtSec = double(dtMs)/1000.0;
   double y = m_waveAmplitudeShift + m_waveGain*sin( m_waveLength*dtSec + m_phaseShift );
   if (y<0)
      y=0;
   else if (y>1.0)
      y=1.0;
   qDebug() << "CEffectWave::calculateIntensity(" << dtSec << ") =" << y;
   return y;
}

QWidget *CEffectWave::buildWidget(QWidget *parent)
{
   QWidget* configWidget = new QWidget( parent );

   auto vlayout = new QVBoxLayout( );

   vlayout->addWidget( new QLabel( "Pahase shift:", configWidget ) );
   QSlider * labelPhaseSlider = new QSlider( configWidget );
   labelPhaseSlider->setMaximum( 100 );
   labelPhaseSlider->setMinimum( 0 );
   labelPhaseSlider->setValue( m_phaseShift*(100/M_2_PI) );
   labelPhaseSlider->setOrientation( Qt::Horizontal );
   vlayout->addWidget( labelPhaseSlider );

   vlayout->addWidget( new QLabel( "Wave length:", configWidget) );
   QSlider * waveLengthSlider = new QSlider( configWidget );
   waveLengthSlider->setMaximum(1000);
   waveLengthSlider->setMinimum(0);
   waveLengthSlider->setValue( m_waveLength*20 );
   waveLengthSlider->setOrientation( Qt::Horizontal );
   vlayout->addWidget( waveLengthSlider );

   vlayout->addWidget( new QLabel( "Wave gain:", configWidget));
   QSlider * waveGain = new QSlider( configWidget );
   waveGain->setMaximum(100);
   waveGain->setMinimum(0);
   waveGain->setValue( m_waveGain*100 );
   waveGain->setOrientation( Qt::Horizontal );
   vlayout->addWidget( waveGain );

   vlayout->addWidget( new QLabel( "Amplitude shift:", configWidget));
   QSlider * waveAmplitudeShift = new QSlider( configWidget );
   waveAmplitudeShift->setMaximum(100);
   waveAmplitudeShift->setMinimum(0);
   waveAmplitudeShift->setValue( (m_waveAmplitudeShift+0.5)*50 );
   waveAmplitudeShift->setOrientation( Qt::Horizontal );
   vlayout->addWidget( waveAmplitudeShift );


   QObject::connect( labelPhaseSlider, &QSlider::valueChanged, [ this ]( int value ){
      m_phaseShift = (value*M_2_PI) / 100.0;
   });

   QObject::connect( waveLengthSlider, &QSlider::valueChanged, [ this ]( int value ){
      m_waveLength = value / 20.0;
   });

   QObject::connect( waveGain, &QSlider::valueChanged, [ this ]( int value ){
      m_waveGain = value / 100.0;
   });

   QObject::connect( waveAmplitudeShift, &QSlider::valueChanged, [ this ]( int value ){
      m_waveAmplitudeShift = -0.5 + value / 50.0;
   });


   configWidget->setLayout( vlayout );

   return configWidget;
}

DECLARE_EFFECT_FACTORY( Wave, CEffectWave )
