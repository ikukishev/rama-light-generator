#ifndef CEffectWave_H
#define CEffectWave_H

#include "timeline/IEffectGenerator.h"

class CEffectWave: public IEffectGenerator
{
public:

   CEffectWave( IEffectGeneratorFactory& afactory )
      : IEffectGenerator( afactory )
   {}

   CEffectWave( IEffectGeneratorFactory& afactory, const QUuid& uuid )
      : IEffectGenerator(afactory, uuid)
   {}

protected:
   virtual QJsonObject toJsonParameters() const override;
   virtual bool parseParameters( const QJsonObject& parameters ) override;
   virtual double calculateIntensity( const SpectrumData& spectrumData  ) override;
   virtual QWidget *buildWidget(QWidget *parent) override;
   virtual std::shared_ptr<IEffectGenerator> makeCopy() const override;

private:

   double m_phaseShift = 0;
   double m_waveLength = 1.0;
   double m_waveGain = 1.0;
   double m_waveAmplitudeShift = 0.0;
};


#endif // CEffectWave
