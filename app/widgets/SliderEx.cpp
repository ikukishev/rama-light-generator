#include "SliderEx.h"

void SliderEx::mousePressEvent(QMouseEvent *event)
{
   emit clicked();
   QSlider::mousePressEvent(event);
}
