#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include "widgets/FloatSliderWidget.h"
#include "spectrograph.h"
#include "CEffectSpectrumBar.h"


const QString cKeyGainValue( "gainValue" );
const QString cKeyFadeValue( "fadeValue" );
const QString cKeyThresholdValue( "thresholdValue" );
const QString cKeySpectrumBarIndex( "spectrumBarIndex" );


QJsonObject CEffectSpectrumBar::toJsonParameters() const
{
   QJsonObject parameters;

   parameters[ cKeyGainValue ] = m_gain;
   parameters[ cKeyFadeValue ] = m_fade;
   parameters[ cKeyThresholdValue ] = m_threshold;
   parameters[ cKeySpectrumBarIndex ] = int(m_spectrumBarIndex);

   return parameters;
}


bool CEffectSpectrumBar::parseParameters(const QJsonObject &parameters)
{
   bool isOk = false;
   if ( parameters.contains( cKeyGainValue )
        && parameters.contains( cKeyFadeValue )
        && parameters.contains( cKeyThresholdValue )
        && parameters.contains( cKeySpectrumBarIndex )  )
   {
      m_fade = parameters[ cKeyFadeValue ].toDouble( 1.0 );
      m_gain = parameters[ cKeyGainValue ].toDouble( 1.0 );
      m_threshold = parameters[ cKeyThresholdValue ].toDouble( 0.0 );
      m_spectrumBarIndex = parameters[ cKeyGainValue ].toInt(1);
      isOk = true;
   }
   return isOk;
}


double CEffectSpectrumBar::calculateIntensity(const SpectrumData &spectrumData)
{

   auto intensityLevel = 0.0;
   if ( spectrumData.spectrum.size() > m_spectrumBarIndex )
   {
      intensityLevel = spectrumData.spectrum[ m_spectrumBarIndex ] * m_gain;
   }

   if ( intensityLevel > 1.0 )
   {
      intensityLevel = 1.0;
   }
   else if ( intensityLevel < 0.0 )
   {
      intensityLevel = 0.0;
   }

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

   if ( lastLevel < intensityLevel && intensityLevel > m_threshold )
   {
       lastLevel = intensityLevel;
   }

   qDebug() << "calculateIntensity";

   if ( nullptr != spectrograph )
   {
      spectrograph->spectrumChanged( spectrumData );
   }

   return lastLevel;
}


QWidget *CEffectSpectrumBar::buildWidget(QWidget *parent)
{

   QWidget* configWidget = new QWidget( parent );
   auto hlayout = new QHBoxLayout( );


   QWidget* configWidgetLeft = new QWidget( configWidget );
   auto vlayout = new QVBoxLayout();


   vlayout->addWidget( new QLabel( "Gain: ", configWidgetLeft ) );
   FloatSliderWidget * gain = new FloatSliderWidget( cMaxGainValue, cMinGainValue, m_gain, configWidgetLeft );
   vlayout->addWidget( gain );

   vlayout->addWidget(new QLabel("Fade: ", configWidgetLeft));
   FloatSliderWidget * fade = new FloatSliderWidget( cMaxFadeValue, cMinFadeValue, m_fade, configWidgetLeft );
   vlayout->addWidget( fade );

   vlayout->addWidget(new QLabel("Threshold: ", configWidgetLeft));
   FloatSliderWidget * threshold = new FloatSliderWidget( cMaxThreshholdValue, cMinThreshholdValue, m_threshold, configWidgetLeft );
   vlayout->addWidget( threshold );

   QObject::connect( gain, &FloatSliderWidget::valueChanged, [ this ]( double value ){
      m_gain = value;
      if ( nullptr != spectrograph )
      {
         spectrograph->setGain( m_gain );
      }
   });

   QObject::connect( fade, &FloatSliderWidget::valueChanged, [ this ]( double value ){
      m_fade = value;
      if ( nullptr != spectrograph )
      {
         spectrograph->setFading( m_fade );
      }
   });

   QObject::connect( threshold, &FloatSliderWidget::valueChanged, [ this ]( double value ){
      m_threshold = value;
      if ( nullptr != spectrograph )
      {
         spectrograph->setMinimumLevel( m_threshold );
      }
   });

   auto widget = new QWidget( configWidgetLeft );
   vlayout->addWidget( widget );

   configWidgetLeft->setMaximumWidth( 400 );
   configWidgetLeft->setLayout( vlayout );
   hlayout->addWidget( configWidgetLeft );


   if ( nullptr != spectrograph )
   {
      delete spectrograph;
      spectrograph = nullptr;
   }
   spectrograph = new Spectrograph( configWidget );

   QObject::connect( spectrograph, &QWidget::destroyed, [this]( QObject* )
   {
      this->spectrograph = nullptr;
   });

   QObject::connect( spectrograph, &Spectrograph::selectedBarChanged, [this]( int index )
   {
      this->m_spectrumBarIndex = index;
   });

   spectrograph->setGain( m_gain );
   spectrograph->setFading( m_fade );
   spectrograph->setMinimumLevel( m_threshold );
   spectrograph->setBarSelected( m_spectrumBarIndex );

   hlayout->addWidget( spectrograph );


   configWidget->setLayout( hlayout );

   return configWidget;
}


DECLARE_EFFECT_FACTORY( SpectrumBar, CEffectSpectrumBar )
