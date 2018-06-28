#-------------------------------------------------
#
# Project created by QtCreator 2014-11-19T10:59:44
#
#-------------------------------------------------

QT       += core gui network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LeapCalibration
TEMPLATE = app


SOURCES += main.cpp \
    screencalibration.cpp \
    calibrationpattern.cpp \
    marker.cpp \
    collector.cpp \
    calibrationtools.cpp \
    mpfit/mpfit.cpp \
    calibrationdata.cpp \
    calibrationserver.cpp

HEADERS  += \
    screencalibration.h \
    calibrationpattern.h \
    marker.h \
    collector.h \
    calibrationtools.h \
    mpfit/mpfit.h \
    calibrationdata.h \
    calibrationserver.h

#FORMS    +=

win32:CONFIG(release, debug|release): LIBS += -L$$(LEAP_SDK)/lib/x86/ -lLeap
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(LEAP_SDK)/lib/x86/ -lLeapd
else:unix: LIBS += -L$$(LEAP_SDK)/lib/x64/ -lLeap

INCLUDEPATH += $$(LEAP_SDK)/include
DEPENDPATH += $$(LEAP_SDK)/include

RESOURCES += \
    resources.qrc
