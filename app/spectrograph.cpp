/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "spectrograph.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTimerEvent>



Spectrograph::Spectrograph(QWidget *parent)
    : QWidget(parent)
    , m_barSelected( NullIndex )
    , m_gain( NoGain )
    , m_minimumLevel( NoMinimumLevel )
    , m_fading( NoFade )
{
    setMinimumHeight(100);
}

Spectrograph::~Spectrograph()
{

}


void Spectrograph::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    const int numBars = m_spectrum.spectrum.size();

    // Highlight region of selected bar
    if (m_barSelected != NullIndex && numBars) {
        QRect regionRect = rect();
        regionRect.setLeft(m_barSelected * rect().width() / numBars);
        regionRect.setWidth(rect().width() / numBars);
        painter.setBrush(Qt::DiagCrossPattern);
        painter.fillRect(regionRect, QColor(202, 202, 64));



        auto dt = m_spectrum.position - m_prev_position;
        auto k = (-1.0 / (1000.0 * m_fading));
        auto b = 1.0;
        double y = k * double(dt) + b;
        double heightReduction = b - y;

        m_current_value -= heightReduction;


        qreal value = m_spectrum.spectrum[m_barSelected] * m_gain;

        if ( value > m_current_value )
        {
            m_current_value = value;
            if ( m_current_value > 1.0 )
            {
                m_current_value = 1.0;
            }
        }

        regionRect.setTop( (1.0-m_current_value)*regionRect.height() );

        if ( m_current_value < m_minimumLevel )
        {
            painter.fillRect(regionRect, QColor(0, 0, 150, 150));
        }
        else
        {
            painter.fillRect(regionRect, QColorConstants::Magenta);
        }


        painter.setBrush(Qt::NoBrush);

        m_prev_position = m_spectrum.position;
    }

    QColor barColor(51, 204, 102);
    QColor clipColor(255, 255, 0);

    // Draw the outline
    const QColor gridColor = barColor.darker();
    QPen gridPen(gridColor);
    painter.setPen(gridPen);
    painter.drawLine(rect().topLeft(), rect().topRight());
    painter.drawLine(rect().topRight(), rect().bottomRight());
    painter.drawLine(rect().bottomRight(), rect().bottomLeft());
    painter.drawLine(rect().bottomLeft(), rect().topLeft());

    QVector<qreal> dashes;
    dashes << 2 << 2;
    gridPen.setDashPattern(dashes);
    painter.setPen(gridPen);

    // Draw vertical lines between bars
    if (numBars) {
        const int numHorizontalSections = numBars;
        QLine line(rect().topLeft(), rect().bottomLeft());
        int w = static_cast<int>( qRound(static_cast<double>(rect().width())/numHorizontalSections));
        for (int i=1; i<numHorizontalSections; ++i) {
            line.translate(w, 0);
            painter.drawLine(line);
        }
    }

    // Draw horizontal lines
    const int numVerticalSections = 10;
    QLine line(rect().topLeft(), rect().topRight());
    for (int i=1; i<numVerticalSections; ++i) {
        line.translate(0, rect().height()/numVerticalSections);
        painter.drawLine(line);
    }

    barColor = barColor.lighter();
    barColor.setAlphaF(0.75);
    clipColor.setAlphaF(0.75);

    // Draw the bars
    if (numBars) {
        // Calculate width of bars and gaps
        const int widgetWidth = rect().width();
        const double barPlusGapWidth = static_cast<double>(widgetWidth) / numBars;
        const double barWidth = 0.8*barPlusGapWidth;
        const double gapWidth = barPlusGapWidth - barWidth;
        const double paddingWidth = widgetWidth - numBars * (barWidth + gapWidth);
        const double leftPaddingWidth = (paddingWidth + gapWidth) / 2;
        const double barHeight = rect().height() - 2 * gapWidth;

        for (int i=0; i<numBars; ++i) {
            qreal value = m_spectrum.spectrum[i];
            if ( value < 0.0 )
                continue;
            if ( value > 1.0 )
                value = 1.0;

            QRect bar = rect();
            bar.setLeft( qRound(rect().left() + leftPaddingWidth + (i * (gapWidth + barWidth))));
            bar.setWidth( qRound(barWidth) );
            bar.setTop(qRound(rect().top() + gapWidth + (1.0 - value) * barHeight));
            bar.setBottom( qRound( rect().bottom() - gapWidth ) );

            QColor color = barColor;

            painter.fillRect(bar, color);
        }
    }

    QPen pen( QColorConstants::Red );
    pen.setWidth( 2 );
    painter.setPen( pen );

    int y = rect().height()*(1.0-m_minimumLevel);
    painter.drawLine(0, y, rect().width(), y);

}

void Spectrograph::mousePressEvent(QMouseEvent *event)
{
    const QPoint pos = event->pos();
    const int index = m_spectrum.spectrum.size() * (pos.x() - rect().left()) / rect().width();
    selectBar(index);
}

void Spectrograph::setFading(double fadeDuration)
{
    if ( fadeDuration < NoFade)
        m_fading = NoFade;
    else
        m_fading = fadeDuration;
}

void Spectrograph::reset()
{
    m_spectrum.spectrum.clear();
    spectrumChanged(m_spectrum);
}

void Spectrograph::spectrumChanged(const SpectrumData &spectrum)
{
    m_spectrum = spectrum;
    update();
}

void Spectrograph::selectBar(int index)
{
    emit selectedBarChanged(index);

    m_barSelected = index;
    update();
}


