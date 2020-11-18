#include "IEffectGenerator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>


const QString cKeyEffectType( "type" );
const QString cKeyEffectUuid( "uuid" );
const QString cKeyEffectNameLabel( "label" );
const QString cKeyEffectDuration( "duration" );
const QString cKeyEffectStartPosition( "startPosition" );
const QString cKeyEffectParameters( "parameters" );

constexpr int cDefaultDuration = 100;


IEffectGeneratorFactory::IEffectGeneratorFactory( const QString &atype )
   : m_type( atype )
{}


std::shared_ptr<IEffectGeneratorFactory> IEffectGeneratorFactory::get( const QString &atype )
{
   std::shared_ptr<IEffectGeneratorFactory> factory = nullptr;

   auto& factories = getGeneratorFactories();
   auto it = factories.find( atype );
   if ( factories.end() != it )
   {
      factory = it->second;
   }

   return factory;
}


std::shared_ptr<IEffectGenerator> IEffectGeneratorFactory::create( const QJsonObject &object )
{
   std::shared_ptr<IEffectGenerator> generator = nullptr;

   if (    object.contains( cKeyEffectType )
        && object.contains( cKeyEffectUuid )
        && object.contains( cKeyEffectStartPosition )
        && object.contains( cKeyEffectDuration )
        && object.contains( cKeyEffectNameLabel ) )
   {
      QString type        = object[ cKeyEffectType ].toString();
      QUuid uuid          = object[ cKeyEffectUuid ].toString();
      int startPosition   = object[ cKeyEffectStartPosition ].toInt(0);
      int duration        = object[ cKeyEffectDuration ].toInt( cDefaultDuration );
      QString label       = object[ cKeyEffectNameLabel ].toString();
      QJsonObject parameters = object[ cKeyEffectParameters ].toObject();

      auto factory = get( type );
      if ( factory )
      {
         auto gen = factory->create( uuid );
         gen->setEffectDuration( duration );
         gen->setEffectStartPosition( startPosition );
         gen->setEffectNameLabel( label );
         if ( gen->parseParameters( parameters ) )
         {
            generator = gen;
         }
      }
   }

   return generator;
}


std::map<QString, std::shared_ptr<IEffectGeneratorFactory> > &IEffectGeneratorFactory::getGeneratorFactories()
{
   static std::map< QString, std::shared_ptr<IEffectGeneratorFactory> > sGeneratorFactories;
   return sGeneratorFactories;
}


IEffectGenerator::IEffectGenerator( IEffectGeneratorFactory &afactory )
   : m_factory(afactory)
   , m_uuid( QUuid::createUuid() )
   , m_effectNameLabel( afactory.type() )
{ }

IEffectGenerator::IEffectGenerator(IEffectGeneratorFactory &afactory, const QUuid &uuid)
   : m_factory( afactory )
   , m_uuid( uuid )
{

}

QJsonObject IEffectGenerator::toJson() const
{
   QJsonObject object;

   object[ cKeyEffectType ] = type();
   object[ cKeyEffectUuid ] = getUuid().toString();
   object[ cKeyEffectNameLabel ] = effectNameLabel();
   object[ cKeyEffectDuration ] = int( effectDuration() );
   object[ cKeyEffectStartPosition ] = int( effectStartPosition() );
   object[ cKeyEffectParameters ] = toJsonParameters();

   return object;
}

double IEffectGenerator::generate(int64_t position , const std::vector<float> &fft)
{
   double intensity = 0.0;
   if ( isPositionActive( position ) )
   {
      intensity = calculateIntensity( position, fft );
   }
   return intensity;
}

QWidget *IEffectGenerator::configurationWidget( QWidget* parent )
{
   QWidget* configWidget = new QWidget( parent );

   auto hlayout = new QHBoxLayout( );

   QWidget* configWidgetLabel = new QWidget( configWidget );
   configWidgetLabel->setMaximumWidth( 300 );

   auto vlayout = new QVBoxLayout( );
   vlayout->addWidget( new QLabel("Type: " + type(), configWidgetLabel));
   vlayout->addWidget( new QLabel("Label: ", configWidgetLabel));
   QLineEdit * labelEdit = new QLineEdit( effectNameLabel(), configWidgetLabel );
   QObject::connect(labelEdit, &QLineEdit::textChanged, [ this ](const QString & alabel){
      m_effectNameLabel = alabel;
   });
   vlayout->addWidget( labelEdit );
   configWidgetLabel->setLayout( vlayout );

   hlayout->addWidget( configWidgetLabel );

   QGroupBox* parametersGroup = new QGroupBox( "Parameters", configWidget );

   auto vGroupLayout = new QVBoxLayout( );

   if ( auto parametersGroupWidget = buildWidget( parametersGroup ) )
   {
      vGroupLayout->addWidget( parametersGroupWidget );
   }

   parametersGroup->setLayout( vGroupLayout );

   hlayout->addWidget( parametersGroup );

   configWidget->setLayout( hlayout );

   return configWidget;
}

bool IEffectGenerator::isPositionActive( int64_t position )  const
{
   return (position >= m_effectStartPosition)
         && (position <= ( m_effectStartPosition + m_effectDuration ));
}
