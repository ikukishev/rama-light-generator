#ifndef FloatSliderWidget_H
#define FloatSliderWidget_H

#include <QWidget>

class SliderEx;
class LabelEx;

class FloatSliderWidget : public QWidget
{
   Q_OBJECT
public:
   explicit FloatSliderWidget(  double max,  double min,  double value, QWidget *parent = nullptr);

   double value() const;
   double minimum() const { return m_minimum; }
   double maximum() const { return m_maximum; }

   void setMaximum( double max );
   void setMinimum( double min );
   void setValue( double value );

signals:
   void valueChanged(double value);
   void clicked();


private:
   void adjust();
   void updateLabel();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;

private:
   SliderEx* slider = nullptr;
   LabelEx* label = nullptr;

   double m_minimum;
   double m_maximum;
};

#endif
