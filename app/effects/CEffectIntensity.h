#ifndef CEFFECTINTENSITY_H
#define CEFFECTINTENSITY_H


#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include "timeline/IEffectGenerator.h"

class CEffectIntensity: public IEffectGenerator
{
public:

   CEffectIntensity( IEffectGeneratorFactory& afactory )
      : IEffectGenerator( afactory )
   {}

   CEffectIntensity( IEffectGeneratorFactory& afactory, const QUuid& uuid )
      : IEffectGenerator(afactory, uuid)
   {}


protected:
   virtual QJsonObject toJsonParameters() const override;
   virtual bool parseParameters( const QJsonObject& parameters ) override;

   virtual double calculateIntensity( const SpectrumData& spectrumData  ) override;

   virtual QWidget *buildWidget(QWidget *parent) override;
   virtual std::shared_ptr<IEffectGenerator> makeCopy() const override;

private:

   double m_intensity = 0.0;
};

#endif // CEFFECTINTENSITY_H
