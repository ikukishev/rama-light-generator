#ifndef CEFFECTTEST_H
#define CEFFECTTEST_H

#include "timeline/IEffectGenerator.h"

class CEffectTest: public IEffectGenerator
{
public:

   CEffectTest( IEffectGeneratorFactory& afactory )
      : IEffectGenerator( afactory )
   {}

   CEffectTest( IEffectGeneratorFactory& afactory, const QUuid& uuid )
      : IEffectGenerator(afactory, uuid)
   {}

protected:
   virtual QJsonObject toJsonParameters() const override;
   virtual bool parseParameters( const QJsonObject& parameters ) override;
   virtual double calculateIntensity( int64_t position, const std::vector<float>& fft ) override;
   virtual QWidget *buildWidget(QWidget *parent) override;

private:

};

#endif // CEFFECTINTENSITY_H
