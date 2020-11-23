
#include <QVBoxLayout>
#include <QLabel>
#include "widgets/FloatSliderWidget.h"
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
   FloatSliderWidget * labelPhaseSlider = new FloatSliderWidget( 2*M_PI, 0.0, m_phaseShift, configWidget );
   vlayout->addWidget( labelPhaseSlider );

   vlayout->addWidget( new QLabel( "Wave length:", configWidget) );
   FloatSliderWidget * waveLengthSlider = new FloatSliderWidget( 50.0, 0, m_waveLength, configWidget );
   vlayout->addWidget( waveLengthSlider );

   vlayout->addWidget( new QLabel( "Wave gain:", configWidget));
   FloatSliderWidget * waveGain = new FloatSliderWidget( 1.0, 0.0, m_waveGain, configWidget );
   vlayout->addWidget( waveGain );

   vlayout->addWidget( new QLabel( "Amplitude shift:", configWidget));
   FloatSliderWidget * waveAmplitudeShift = new FloatSliderWidget( 1.5, -0.5, m_waveAmplitudeShift, configWidget );
   vlayout->addWidget( waveAmplitudeShift );


   QObject::connect( labelPhaseSlider, &FloatSliderWidget::valueChanged, [ this ]( double value ){ m_phaseShift = value; });
   QObject::connect( waveLengthSlider, &FloatSliderWidget::valueChanged, [ this ]( double value ){ m_waveLength = value; });
   QObject::connect( waveGain, &FloatSliderWidget::valueChanged, [ this ]( double value ){ m_waveGain = value; });
   QObject::connect( waveAmplitudeShift, &FloatSliderWidget::valueChanged, [ this ]( double value ){ m_waveAmplitudeShift = value; });


   configWidget->setLayout( vlayout );

   return configWidget;
}

DECLARE_EFFECT_FACTORY( Wave, CEffectWave )
