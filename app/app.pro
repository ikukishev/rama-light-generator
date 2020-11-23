TEMPLATE = app

TARGET = spectrum

QT       += widgets serialport

SOURCES  += channelconfigurator.cpp \
            CConfiguration.cpp \
            ceffecteditorwidget.cpp \
            clightsequence.cpp \
            clorserialctrl.cpp \
            csequensegenerator.cpp \
            effects/CEffectFade.cpp \
            effects/CEffectIntensity.cpp \
            effects/CEffectMaxLevel.cpp \
            effects/CEffectSpectrumBar.cpp \
            effects/CEffectWave.cpp \
            main.cpp \
            mainwindow.cpp \
            qbassaudiofile.cpp \
            spectrograph.cpp \
            timeline/CTimeLineChannel.cpp \
            timeline/CTimeLineEffect.cpp \
            timeline/CTimeLineIndicator.cpp \
            timeline/CTimeLinePosition.cpp \
            timeline/CTimeLineView.cpp \
            timeline/IEffectGenerator.cpp \
            timeline/ITimeLineTrackView.cpp \
            widgets/FloatSliderWidget.cpp \
            widgets/LabelEx.cpp \
            widgets/SliderEx.cpp

HEADERS  += channelconfigurator.h \
            CConfiguration.h \
            SpectrumData.h \
            ceffecteditorwidget.h \
            clightsequence.h \
            clorserialctrl.h \
            constants.h \
            csequensegenerator.h \
            effects/CEffectFade.h \
            effects/CEffectIntensity.h \
            effects/CEffectMaxLevel.h \
            effects/CEffectSpectrumBar.h \
            effects/CEffectWave.h \
            mainwindow.h \
            qbassaudiofile.h \
            spectrograph.h \
            timeline/CTimeLineChannel.h \
            timeline/CTimeLineEffect.h \
            timeline/CTimeLineIndicator.h \
            timeline/CTimeLinePosition.h \
            timeline/CTimeLineView.h \
            timeline/IEffectGenerator.h \
            timeline/ITimeLineTrackView.h \
            widgets/FloatSliderWidget.h \
            widgets/LabelEx.h \
            widgets/SliderEx.h

INCLUDEPATH += ../3rdparty/
INCLUDEPATH += ../3rdparty/bass24-linux

RESOURCES += spectrum.qrc
RESOURCES +=  ../3rdparty/qdarkstyle/style.qrc

CONFIG += install_ok  # Do not cargo-cult this!
CONFIG += c++14

FORMS += \
    channelconfigurator.ui \
    mainwindow.ui


win32 {
    LIBS += -L$$PWD/../3rdparty/bass24/x64
    LIBS += -lbass
} else {
    linux-g++*: {
        LIBS += -L$$PWD/../3rdparty/bass24-linux/x64
        LIBS += -lbass
        # Provide relative path from application to fftreal library
        QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN
    }
}
