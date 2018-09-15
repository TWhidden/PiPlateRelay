QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG += exceptions
CONFIG += thread

TARGET = blinkLib
TEMPLATE = lib


# The following define makes your compiler emit warnings if you use
# any feature of Qt whi-lwiringPich as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
           spibase.cpp \
           relayplate.cpp \
           daqc2plate.cpp

LIBS += -lwiringPi -lcrypt -lrt


QMAKE_INCDIR +=  $$[QT_SYSROOT]/usr/local/include

target.path = /home/pi/blink
INSTALLS += target

INCLUDEPATH +=  $$[QT_SYSROOT]/usr/local/include


HEADERS += \
    spibase.h \
    relayplate.h \
    daqc2plate.h \
    


