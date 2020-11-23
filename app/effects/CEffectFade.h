#ifndef CEFFECTFADE_H
#define CEFFECTFADE_H

#include "timeline/IEffectGenerator.h"
#include "constants.h"

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
   virtual double calculateIntensity( const SpectrumData& spectrumData  ) override;
   virtual QWidget *buildWidget(QWidget *parent) override;

private:

   double m_start_intensity = 0.0;
   double m_end_intensity = 1.0;
};


#endif // CEFFECTFADEUP
