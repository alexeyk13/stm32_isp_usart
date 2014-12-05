#-------------------------------------------------
#
# Project created by QtCreator 2014-11-14T11:22:31
#
#-------------------------------------------------

QT       += core gui widgets serialport

TARGET = stm32_isp_usart
TEMPLATE = app


SOURCES += main.cpp\
	 mainwindow.cpp \
    comm.cpp

HEADERS  += mainwindow.h \
    comm.h \
    common.h \
    config.h \
    delay.h \
    error.h \
    proto.h

FORMS    += mainwindow.ui

linux*{
LIBS += -ludev
DEFINES += HAVE_LIBUDEV
}

win32*{
LIBS += -lsetupapi
}

CONFIG += exceptions
