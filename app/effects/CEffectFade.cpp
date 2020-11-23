
#include <QVBoxLayout>
#include <QLabel>

#include "widgets/FloatSliderWidget.h"
#include "CEffectFade.h"

const QString cKeyStartIntensityValue( "startIntensityValue" );
const QString cKeyEndIntensityValue( "endIntensityValue" );


QJsonObject CEffectFade::toJsonParameters() const
{
   QJsonObject parameters;

   parameters[ cKeyStartIntensityValue ] = m_start_intensity;
   parameters[ cKeyEndIntensityValue ]   = m_end_intensity;

   return parameters;
}

bool CEffectFade::parseParameters(const QJsonObject &parameters)
{
   bool isOk = false;
   if ( parameters.contains( cKeyStartIntensityValue )
        && parameters.contains( cKeyEndIntensityValue )  )
   {
      if ( parameters[ cKeyStartIntensityValue ].isDouble()
           && parameters[ cKeyEndIntensityValue ].isDouble() )
      {
         m_start_intensity = parameters[ cKeyStartIntensityValue ].toDouble( );
         m_end_intensity = parameters[ cKeyEndIntensityValue ].toDouble( );
         isOk = true;
      }
   }
   return isOk;
}

double CEffectFade::calculateIntensity(const SpectrumData &spectrumData)
{
   double& y0 = m_start_intensity;
   double& y1 = m_end_intensity;
   double x0 = effectStartPosition();
   double x1 = effectDuration() + effectStartPosition();

   double y = y0 + (spectrumData.position - x0)*(y1 - y0)/(x1 - x0);
   return y;
}

QWidget *CEffectFade::buildWidget(QWidget *parent)
{
   QWidget* configWidget = new QWidget( parent );

   auto vlayout = new QVBoxLayout( );

   vlayout->addWidget( new QLabel("Start intensity: ", configWidget) );
   FloatSliderWidget * startIntensity = new FloatSliderWidget( cMaxIntensity, cMinIntensity, m_start_intensity, configWidget );
   vlayout->addWidget( startIntensity );

   vlayout->addWidget( new QLabel("End intensity: ", configWidget));
   FloatSliderWidget * endIntensity = new FloatSliderWidget( cMaxIntensity, cMinIntensity, m_end_intensity, configWidget );
   vlayout->addWidget( endIntensity );

   QObject::connect( startIntensity, &FloatSliderWidget::valueChanged, [ this ]( double value ){
      m_start_intensity = value;
   });

   QObject::connect( endIntensity, &FloatSliderWidget::valueChanged, [ this ]( double value ){
      m_end_intensity = value;
   });

   configWidget->setLayout( vlayout );

   return configWidget;
}

DECLARE_EFFECT_FACTORY( Fade, CEffectFade )
