include(../spectrum.pri)

TEMPLATE = app

TARGET = spectrum

QT       += multimedia widgets

SOURCES  += channelconfigurator.cpp \
            fft-base/qbassaudiofile.cpp \
            fft-base/spectrograph.cpp \
            main.cpp \
            mainwindow.cpp

HEADERS  += channelconfigurator.h \
            fft-base/qbassaudiofile.h \
            fft-base/spectrograph.h \
            mainwindow.h

fftreal_dir = ../3rdparty/fftreal

INCLUDEPATH += $${fftreal_dir}
INCLUDEPATH += ../3rdparty/bass24-linux

RESOURCES = fft-base/spectrum.qrc

LIBS += -L..$${spectrum_build_dir}
LIBS += -L$$PWD/../3rdparty/bass24-linux/x64
LIBS += -lfftreal -lbass

target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/spectrum
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!

# Deployment

DESTDIR = ..$${spectrum_build_dir}

linux-g++*: {
    # Provide relative path from application to fftreal library
    QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN
}

# include(../../shared/shared.pri)

FORMS += \
    channelconfigurator.ui \
    mainwindow.ui
