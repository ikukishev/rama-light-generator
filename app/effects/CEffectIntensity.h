#ifndef CEFFECTINTENSITY_H
#define CEFFECTINTENSITY_H


#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include "timeline/IEffectGenerator.h"

class CEffectInten: public IEffectGenerator
{
public:

   CEffectInten( IEffectGeneratorFactory& afactory )
      : IEffectGenerator( afactory )
   {}

   CEffectInten( IEffectGeneratorFactory& afactory, const QUuid& uuid )
      : IEffectGenerator(afactory, uuid)
   {}

   virtual ~CEffectInten();

protected:
   virtual QJsonObject toJsonParameters() const override;
   virtual bool parseParameters( const QJsonObject& parameters ) override;
   virtual double calculateIntensity( int64_t position, const std::vector<float>& fft ) override;
   virtual QWidget *buildWidget(QWidget *parent) override;

private:

   double m_intensity = 0.0;
};


class CEffectGeneratorFactoryIntensity: public IEffectGeneratorFactory
{
public:
   CEffectGeneratorFactoryIntensity(): IEffectGeneratorFactory( "Intensity" ) {}

   virtual std::shared_ptr< IEffectGenerator > create() override
   {
      auto ptr = std::make_shared<CEffectInten>( *this );
      qDebug() << "create" << type() << "ptr:" << (void*)ptr.get();
      return ptr;
   }

   virtual std::shared_ptr< IEffectGenerator > create( const QUuid& uuid ) override
   {
      return std::make_shared<CEffectInten>( *this, uuid );
   }
};

#endif // CEFFECTINTENSITY_H
