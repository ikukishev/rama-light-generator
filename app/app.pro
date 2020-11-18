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
            timeline/ITimeLineTrackView.cpp

HEADERS  += channelconfigurator.h \
            CConfiguration.h \
            ceffecteditorwidget.h \
            clightsequence.h \
            clorserialctrl.h \
            csequensegenerator.h \
            effects/CEffectFade.h \
            effects/CEffectIntensity.h \
            mainwindow.h \
            qbassaudiofile.h \
            spectrograph.h \
            timeline/CTimeLineChannel.h \
            timeline/CTimeLineEffect.h \
            timeline/CTimeLineIndicator.h \
            timeline/CTimeLinePosition.h \
            timeline/CTimeLineView.h \
            timeline/IEffectGenerator.h \
            timeline/ITimeLineTrackView.h

INCLUDEPATH += ../3rdparty/
INCLUDEPATH += ../3rdparty/bass24-linux

RESOURCES = fft-base/spectrum.qrc
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
