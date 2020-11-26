#include "LabelEx.h"

void LabelEx::emitClick()
{
    emit clicked();
}

void LabelEx::mousePressEvent(QMouseEvent *event)
{
   emit clicked();
   QLabel::mousePressEvent(event);
}
