#include "CEffectIntensity.h"

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

double CEffectIntensity::calculateIntensity(int64_t position, const std::vector<float> &fft)
{
    return m_intensity;
}

QWidget *CEffectIntensity::buildWidget(QWidget *parent)
{
    QWidget* configWidget = new QWidget( parent );

    auto vlayout = new QVBoxLayout( );
    auto label = new QLabel("Intensity: ", configWidget);
    label->setMaximumHeight( labelHeight );
    vlayout->addWidget( label );

    QSlider * intensity = new QSlider( configWidget );
    intensity->setMaximum(100);
    intensity->setValue( m_intensity * 100 );
    intensity->setMinimum(0);
    intensity->setOrientation( Qt::Horizontal );
    intensity->setMaximumHeight( labelHeight );

    QObject::connect(intensity, &QSlider::valueChanged, [ this ]( int value ){
        m_intensity = double(value)/100.0;
    });

    vlayout->addWidget( intensity );
    configWidget->setLayout( vlayout );

    return configWidget;
}

DECLARE_EFFECT_FACTORY( Intensity, CEffectIntensity )
