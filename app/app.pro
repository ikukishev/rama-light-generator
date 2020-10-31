include(../spectrum.pri)

TEMPLATE = app

TARGET = spectrum

QT       += multimedia widgets

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

fftreal_dir = ../3rdparty/fftreal

INCLUDEPATH += $${fftreal_dir}
INCLUDEPATH += ../3rdparty/bass24-linux
INCLUDEPATH += ../3rdparty/

RESOURCES = fft-base/spectrum.qrc

LIBS += -L..$${spectrum_build_dir}
LIBS += -L$$PWD/../3rdparty/bass24-linux/x64
LIBS += -lbass

target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/spectrum
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
CONFIG += c++17
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
