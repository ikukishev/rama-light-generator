
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

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

double CEffectFade::calculateIntensity( int64_t position, const std::vector<float>& )
{
   double& y0 = m_start_intensity;
   double& y1 = m_end_intensity;
   double x0 = effectStartPosition();
   double x1 = effectDuration() + effectStartPosition();

   double y = y0 + (position - x0)*(y1 - y0)/(x1 - x0);
   return y;
}

QWidget *CEffectFade::buildWidget(QWidget *parent)
{
   QWidget* configWidget = new QWidget( parent );

   auto vlayout = new QVBoxLayout( );

   vlayout->addWidget( new QLabel("Start intensity: ", configWidget));
   QSlider * startIntensity = new QSlider( configWidget );
   startIntensity->setMaximum( 100 );
   startIntensity->setMinimum( 0 );
   startIntensity->setValue( m_start_intensity*100 );
   vlayout->addWidget( startIntensity );

   vlayout->addWidget( new QLabel("End intensity: ", configWidget));
   QSlider * endIntensity = new QSlider( configWidget );
   endIntensity->setMaximum(100);
   endIntensity->setMinimum(0);
   endIntensity->setValue( m_end_intensity*100 );
   vlayout->addWidget( endIntensity );

   QObject::connect( startIntensity, &QSlider::valueChanged, [ this ]( int value ){
      m_start_intensity = double(value) / 100.0;
   });

   QObject::connect( endIntensity, &QSlider::valueChanged, [ this ]( int value ){
      m_end_intensity = double(value) / 100.0;
   });

   configWidget->setLayout( vlayout );

   return configWidget;
}

