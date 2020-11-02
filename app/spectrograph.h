
#ifndef SPECTROGRAPH_H
#define SPECTROGRAPH_H

#include "qbassaudiofile.h"

#include <QWidget>

/**
 * Widget which displays a spectrograph showing the frequency spectrum
 * of the window of audio samples most recently analyzed by the Engine.
 */




class Spectrograph : public QWidget
{
    Q_OBJECT
public:
    static constexpr int NullIndex = -1;
    static constexpr double NoGain = 1.0;
    static constexpr double NoMinimumLevel = 0.0;
    static constexpr double NoFade = 0.01;

public:
    explicit Spectrograph(QWidget *parent = 0);
    ~Spectrograph();

    void setParams(int numBars, qreal lowFreq, qreal highFreq);

    // QWidget
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void setBarSelected( int index) { m_barSelected = index; };
    void setGain( double gain ) { m_gain = gain; }
    void setMinimumLevel( double level ) { m_minimumLevel = level; }
    void setFading( double fadeDuration );

signals:
    void selectedBarChanged(int index);

public slots:
    void reset();
    void spectrumChanged(const SpectrumData &spectrum);

private:
    void selectBar(int index);

private:

    int                 m_barSelected;
    SpectrumData        m_spectrum;
    double              m_gain;
    double              m_minimumLevel;
    double              m_fading;

    uint64_t            m_prev_position = 0;
    float               m_current_value = 0.0f;
};

#endif // SPECTROGRAPH_H
