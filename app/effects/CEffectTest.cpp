
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

#include "CEffectTest.h"



QJsonObject CEffectTest::toJsonParameters() const
{
   QJsonObject parameters;

   return parameters;
}

bool CEffectTest::parseParameters(const QJsonObject &parameters)
{
   qDebug() << "CEffectTest::parseParameters";
   return true;
}

double CEffectTest::calculateIntensity( int64_t position, const std::vector<float>& fft )
{
   qDebug() << "CEffectTest::calculateIntensity";
   return 0;
}

QWidget *CEffectTest::buildWidget(QWidget *parent)
{
   qDebug() << "CEffectTest::buildWidget";
   QWidget* configWidget = new QWidget( parent );
   return configWidget;
}

