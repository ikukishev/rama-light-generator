include(spectrum.pri)

TEMPLATE = subdirs

# Ensure that library is built before application
CONFIG  += ordered

SUBDIRS += app

TARGET = spectrum

EXAMPLE_FILES += \
    README.txt \
    TODO.txt
