#ifndef SLIDEREX_H
#define SLIDEREX_H

#include <QSlider>


class SliderEx : public QSlider
{
    Q_OBJECT
public:
    explicit SliderEx(QWidget *parent = nullptr)
        : QSlider( parent )
    {}

    explicit SliderEx(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSlider( orientation, parent )
    {}

signals:
    void clicked();

protected:

    virtual void mousePressEvent(QMouseEvent* event) override;

};



#endif // SLIDEREX_H
