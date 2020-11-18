#ifndef CEFFECTFADE_H
#define CEFFECTFADE_H

#include "timeline/IEffectGenerator.h"

class CEffectFade: public IEffectGenerator
{
public:

   CEffectFade( IEffectGeneratorFactory& afactory )
      : IEffectGenerator( afactory )
   {}

   CEffectFade( IEffectGeneratorFactory& afactory, const QUuid& uuid )
      : IEffectGenerator(afactory, uuid)
   {}

protected:
   virtual QJsonObject toJsonParameters() const override;
   virtual bool parseParameters( const QJsonObject& parameters ) override;
   virtual double calculateIntensity( int64_t position, const std::vector<float>& fft ) override;
   virtual QWidget *buildWidget(QWidget *parent) override;

private:

   double m_start_intensity = 0.0;
   double m_end_intensity = 1.0;
};



class CEffectGeneratorFactoryFade: public IEffectGeneratorFactory
{
public:
   CEffectGeneratorFactoryFade(): IEffectGeneratorFactory( "Fade" ) {}

   virtual std::shared_ptr< IEffectGenerator > create() override
   {
      auto ptr = std::make_shared<CEffectFade>( *this );
      qDebug() << "create" << type() << "ptr:" << (void*)ptr.get();
      return ptr;
   }

   virtual std::shared_ptr< IEffectGenerator > create( const QUuid& uuid ) override
   {
      return std::make_shared<CEffectFade>( *this, uuid );
   }
};


#endif // CEFFECTFADEUP
