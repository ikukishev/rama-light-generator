
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>


#include "CEffectMaxLevel.h"


const QString cKeyGainValue( "gainValue" );
const QString cKeyFadeValue( "fadeValue" );


QJsonObject CEffectMaxLevel::toJsonParameters() const
{
   QJsonObject parameters;

   parameters[ cKeyGainValue ] = m_gain;
   parameters[ cKeyFadeValue ] = m_fade;

   return parameters;
}

bool CEffectMaxLevel::parseParameters(const QJsonObject &parameters)
{
   bool isOk = false;
   if ( parameters.contains( cKeyGainValue )
        && parameters.contains( cKeyFadeValue )  )
   {
      if ( parameters[ cKeyGainValue ].isDouble()
           && parameters[ cKeyFadeValue ].isDouble() )
      {
         m_fade = parameters[ cKeyFadeValue ].toDouble( );
         m_gain = parameters[ cKeyGainValue ].toDouble( );
         isOk = true;
      }
   }
   return isOk;
}

double CEffectMaxLevel::calculateIntensity(const SpectrumData &spectrumData)
{
    float maxLevel = 0;
    for ( auto& level : spectrumData.spectrum )
    {
        if ( level > maxLevel )
        {
            maxLevel = level;
        }
    }

    maxLevel = maxLevel * m_gain;
    if ( maxLevel > 1.0 )
        maxLevel = 1.0;
    else if ( maxLevel < 0.0 )
        maxLevel = 0.0;

    auto dt = spectrumData.position - lastPosition;
    if (dt < 0)
    {
        dt=0;
    }
    lastPosition = spectrumData.position;

   auto k = (-1.0 / (1000.0 * ( m_fade < 0.1 ? 0.1 : m_fade )));
   auto b = 1.0;
   double y = k * double(dt) + b;
   double intensityReduction = b - y;

   lastLevel -= intensityReduction;

   if ( lastLevel < maxLevel )
   {
       lastLevel = maxLevel;
   }

   return lastLevel;
}

QWidget *CEffectMaxLevel::buildWidget(QWidget *parent)
{

   QWidget* configWidget = new QWidget( parent );

   auto vlayout = new QVBoxLayout( );

   auto gainLabel =  new QLabel("Gain: ", configWidget);
   gainLabel->setMaximumHeight( 50 );

   vlayout->addWidget(gainLabel);

   QSlider * gain = new QSlider( configWidget );
   gain->setMaximumHeight( 40 );
   gain->setOrientation( Qt::Horizontal );
   gain->setMaximum( 100 );
   gain->setMinimum( 0 );
   gain->setValue( m_gain * 20 );
   vlayout->addWidget( gain );

   auto fadeLabel =  new QLabel("Fade: ", configWidget);
   fadeLabel->setMaximumHeight( 50 );

   vlayout->addWidget(fadeLabel);

   QSlider * fade = new QSlider( configWidget );
   fade->setMaximumHeight( 40 );
   fade->setOrientation( Qt::Horizontal );
   fade->setMaximum(100);
   fade->setMinimum(0);
   fade->setValue( m_fade*20 );
   vlayout->addWidget( fade );

   QObject::connect( gain, &QSlider::valueChanged, [ this ]( int value ){
      m_gain = double(value) / 20.0;
   });

   QObject::connect( fade, &QSlider::valueChanged, [ this ]( int value ){
      m_fade = double(value) / 20.0;
   });

   auto widget = new QWidget( configWidget );
   vlayout->addWidget( widget );

   configWidget->setLayout( vlayout );

   return configWidget;
}

DECLARE_EFFECT_FACTORY( MaxLevel, CEffectMaxLevel )
