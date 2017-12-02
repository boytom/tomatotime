#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T22:38:05
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tomato
TEMPLATE = app

#CONFIG += debug

unix:QMAKE_CXXFLAGS += -std=gnu++14 -finput-charset=gb18030 -fexec-charset=utf-8
unix:QMAKE_CFLAGS += -std=gnu11 -finput-charset=gb18030 -fexec-charset=utf-8
unix:QMAKE_LFLAGS += -Wl,-rpath,/home/whg/soft/gcc/lib64
unix:QT += x11extras

win32: {
QMAKE_CXXFLAGS_DEBUG +=-D_DEBUG #microsoft crt debug need this macro being defined
QMAKE_CFLAGS_DEBUG +=-D_DEBUG #microsoft crt debug need this macro being def
DEFINES += _UNICODE UNICODE
QMAKE_LFLAGS +=/ENTRY:"wmainCRTStartup"
QMAKE_CXXFLAGS += -DUNICODE -D_UNICODE -Zc:wchar_t -execution-charset:gb18030 -source-charset:gb2312
QMAKE_LIBS += user32.lib gdi32.lib
RC_FILE=icon-tomato.rc
}


SOURCES += main.cpp\
        mainwindow.cpp \
    lunar.c \
    widget.cpp

HEADERS  += mainwindow.h \
    lunar.h \
    widget.h

FORMS    += mainwindow.ui \
    widget.ui

RESOURCES += \
    tomato.qrc
