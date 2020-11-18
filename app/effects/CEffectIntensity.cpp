#include "CEffectIntensity.h"

const QString cKeyIntensityValue( "intensityValue" );



CEffectInten::~CEffectInten()
{
   qDebug() << "CEffectInten::~CEffectInten()";
}

QJsonObject CEffectInten::toJsonParameters() const
{
   QJsonObject parameters;

   parameters[ cKeyIntensityValue ] = m_intensity;

   return parameters;
}

bool CEffectInten::parseParameters(const QJsonObject &parameters)
{
   qDebug() << "CEffectInten::parseParameters";
   bool isOk = false;
   if ( parameters.contains( cKeyIntensityValue ) )
   {
      if ( parameters[ cKeyIntensityValue ].isDouble() )
      {
         m_intensity = parameters[ cKeyIntensityValue ].toDouble( );
         isOk = true;
      }
   }
   return isOk;
}

double CEffectInten::calculateIntensity(int64_t position, const std::vector<float> &fft)
{
   qDebug() << "CEffectIntensity::calculateIntensity";
   return m_intensity;
}

QWidget *CEffectInten::buildWidget(QWidget *parent)
{
   qDebug() << "CEffectIntensity::buildWidget";
   QWidget* configWidget = new QWidget( parent );

   auto vlayout = new QVBoxLayout( );
   vlayout->addWidget( new QLabel("Intensity: ", configWidget));

   QSlider * intensity = new QSlider( configWidget );
   intensity->setMaximum(100);
   intensity->setMinimum(0);

   QObject::connect(intensity, &QSlider::valueChanged, [ this ]( int value ){
      m_intensity = double(value)/100.0;
   });

   vlayout->addWidget( intensity );
   configWidget->setLayout( vlayout );

   return configWidget;
}
