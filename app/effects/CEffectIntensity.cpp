#include "CEffectIntensity.h"
#include "widgets/FloatSliderWidget.h"
#include "constants.h"

const QString cKeyIntensityValue( "intensityValue" );


QJsonObject CEffectIntensity::toJsonParameters() const
{
    QJsonObject parameters;

    parameters[ cKeyIntensityValue ] = m_intensity;

    return parameters;
}

bool CEffectIntensity::parseParameters(const QJsonObject &parameters)
{
    bool isOk = false;
    if ( parameters.contains( cKeyIntensityValue ) )
    {
        if ( parameters[ cKeyIntensityValue ].isDouble() )
        {
            m_intensity = parameters[ cKeyIntensityValue ].toDouble( );
            isOk = true;
        }
    }
    return isOk;
}

double CEffectIntensity::calculateIntensity(const SpectrumData &)
{
    return m_intensity;
}

QWidget *CEffectIntensity::buildWidget(QWidget *parent)
{
    QWidget* configWidget = new QWidget( parent );

    auto vlayout = new QVBoxLayout( );
    vlayout->addWidget( new QLabel("Intensity: ", configWidget) );

    FloatSliderWidget * intensity = new FloatSliderWidget( cMaxIntensity, cMinIntensity, m_intensity, configWidget );

    QObject::connect(intensity, &FloatSliderWidget::valueChanged, [ this ]( double value ){ m_intensity = value; });

    vlayout->addWidget( intensity );
    configWidget->setLayout( vlayout );

    return configWidget;
}

std::shared_ptr<IEffectGenerator> CEffectIntensity::makeCopy() const
{
    return std::make_shared<CEffectIntensity>(*this);
}

DECLARE_EFFECT_FACTORY( Intensity, CEffectIntensity )
