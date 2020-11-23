#ifndef CEFFECTFADE_H
#define CEFFECTFADE_H

#include "constants.h"
#include "timeline/IEffectGenerator.h"

class CEffectMaxLevel: public IEffectGenerator
{
public:

   CEffectMaxLevel( IEffectGeneratorFactory& afactory )
      : IEffectGenerator( afactory )
   {}

   CEffectMaxLevel( IEffectGeneratorFactory& afactory, const QUuid& uuid )
      : IEffectGenerator(afactory, uuid)
   {}

protected:
   virtual QJsonObject toJsonParameters() const override;
   virtual bool parseParameters( const QJsonObject& parameters ) override;
   virtual double calculateIntensity( const SpectrumData& spectrumData  ) override;
   virtual QWidget *buildWidget(QWidget *parent) override;

private:

   double m_gain = cDefaultGainValue;
   double m_fade = cDefaultFadeValue;
   int64_t lastPosition = 0;
   double lastLevel = 0;
};


#endif // CEFFECTFADEUP
