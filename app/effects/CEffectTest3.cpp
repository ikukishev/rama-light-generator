
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

#include "CEffectTest3.h"


QJsonObject CEffectTest3::toJsonParameters() const
{
   QJsonObject parameters;

   return parameters;
}

bool CEffectTest3::parseParameters(const QJsonObject &parameters)
{
   qDebug() << "CEffectTest3::parseParameters";
   return true;
}

double CEffectTest3::calculateIntensity( int64_t position, const std::vector<float>& fft )
{
   qDebug() << "CEffectTest3::calculateIntensity";
   return 0;
}

QWidget *CEffectTest3::buildWidget(QWidget *parent)
{
   qDebug() << "CEffectTest3::buildWidget";
   QWidget* configWidget = new QWidget( parent );
   return configWidget;
}
