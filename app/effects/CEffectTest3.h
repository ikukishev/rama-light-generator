#ifndef CEFFECTTEST3_H
#define CEFFECTTES3T_H

#include "timeline/IEffectGenerator.h"

class CEffectTest3: public IEffectGenerator
{
public:

   CEffectTest3( IEffectGeneratorFactory& afactory )
      : IEffectGenerator( afactory )
   {}

   CEffectTest3( IEffectGeneratorFactory& afactory, const QUuid& uuid )
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
