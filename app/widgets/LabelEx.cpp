#include "LabelEx.h"

void LabelEx::mousePressEvent(QMouseEvent *event)
{
   emit clicked();
   QLabel::mousePressEvent(event);
}
