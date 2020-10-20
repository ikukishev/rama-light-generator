
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
    explicit Spectrograph(QWidget *parent = 0);
    ~Spectrograph();

    void setParams(int numBars, qreal lowFreq, qreal highFreq);

    // QWidget
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void infoMessage(int index);

public slots:
    void reset();
    void spectrumChanged(const SpectrumData &spectrum);

private:
    void selectBar(int index);

private:

    int                 m_barSelected;
    int                 m_timerId;
    SpectrumData   m_spectrum;
};

#endif // SPECTROGRAPH_H
