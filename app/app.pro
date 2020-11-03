TEMPLATE = app

TARGET = spectrum

QT       += widgets

SOURCES  += channelconfigurator.cpp \
            CConfiguration.cpp \
            clightsequence.cpp \
            csequensegenerator.cpp \
            main.cpp \
            mainwindow.cpp \
            qbassaudiofile.cpp \
            spectrograph.cpp

HEADERS  += channelconfigurator.h \
            CConfiguration.h \
            clightsequence.h \
            csequensegenerator.h \
            mainwindow.h \
            qbassaudiofile.h \
            spectrograph.h

INCLUDEPATH += ../3rdparty/
INCLUDEPATH += ../3rdparty/bass24-linux

RESOURCES = fft-base/spectrum.qrc

CONFIG += install_ok  # Do not cargo-cult this!
CONFIG += c++17

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
