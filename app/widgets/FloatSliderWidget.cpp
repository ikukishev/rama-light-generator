#include <QHBoxLayout>
#include <stdio.h>
#include "FloatSliderWidget.h"
#include "LabelEx.h"
#include "SliderEx.h"


constexpr int cSlideMaximumValue = 4096;

FloatSliderWidget::FloatSliderWidget(  double max,  double min,  double val, QWidget *parent )
   : QWidget(parent)
   , m_minimum(min)
   , m_maximum(max)
{
   assert( min < max );
   adjust( );
   setValue( val );
}

double FloatSliderWidget::value() const
{
   return m_minimum + slider->value() * (m_maximum - m_minimum) / cSlideMaximumValue;
}

void FloatSliderWidget::setMaximum(double max)
{
   assert( minimum() < max);
   auto val = value();
   m_maximum = max;
   if ( val > max )
      setValue( max );
}

void FloatSliderWidget::setMinimum(double min)
{
   assert( min < maximum() );
   auto val = value();
   m_minimum = min;
   if ( val < min )
      setValue( min );
}

void FloatSliderWidget::setValue(double val)
{
   if ( val < minimum() ) val = minimum();
   if ( val > maximum() ) val = maximum();

   slider->setValue( (val - m_minimum) * (cSlideMaximumValue / (m_maximum - m_minimum) ) );
   updateLabel();
}

void FloatSliderWidget::adjust()
{
   QHBoxLayout* l = new QHBoxLayout();
   l->setContentsMargins(0,0,0,0);
   setLayout( l );

   slider = new SliderEx( this );
   slider->setMinimum(0);
   slider->setMaximum(cSlideMaximumValue);
   slider->setOrientation( Qt::Horizontal );
   connect( slider, &SliderEx::valueChanged, [ this ]( int ) {
      updateLabel();
      emit valueChanged( value() );
   } );
   connect( slider, &SliderEx::clicked, [ this ]( ) {
      emit clicked();
   } );
   l->addWidget( slider );

   label = new LabelEx( this );
   label->setMaximumWidth( 40 );
   l->addWidget( label );
}


void FloatSliderWidget::updateLabel()
{
   char buf[10] ={0};
   snprintf(buf, 10, "%5.2f", value());
   label->setText( QString(buf) );
}


void FloatSliderWidget::mousePressEvent( QMouseEvent *event )
{
   emit clicked();
   QWidget::mousePressEvent( event );
}
