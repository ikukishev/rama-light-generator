
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include "widgets/FloatSliderWidget.h"


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

    auto dt = int64_t(spectrumData.position) - lastPosition;
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

   FloatSliderWidget * gain = new FloatSliderWidget( cMaxGainValue, cMinGainValue, m_gain, configWidget );
   vlayout->addWidget( gain );

   auto fadeLabel =  new QLabel("Fade: ", configWidget);
   fadeLabel->setMaximumHeight( 50 );

   vlayout->addWidget(fadeLabel);

   FloatSliderWidget * fade = new FloatSliderWidget( cMaxFadeValue, cMinFadeValue, m_fade, configWidget );
   vlayout->addWidget( fade );

   QObject::connect( gain, &FloatSliderWidget::valueChanged, [ this ]( double value ){ m_gain = value; });
   QObject::connect( fade, &FloatSliderWidget::valueChanged, [ this ]( double value ){ m_fade = value; });

   auto widget = new QWidget( configWidget );
   vlayout->addWidget( widget );

   configWidget->setLayout( vlayout );

   return configWidget;
}

std::shared_ptr<IEffectGenerator> CEffectMaxLevel::makeCopy() const
{
    return std::make_shared<CEffectMaxLevel>(*this);
}

DECLARE_EFFECT_FACTORY( MaxLevel, CEffectMaxLevel )
