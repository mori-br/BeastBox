#-------------------------------------------------
#
# Project created by QtCreator 2016-01-17T22:01:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BeastBox
TEMPLATE = app

SOURCES += main.cpp\
        dialog.cpp \
    hidhandler.cpp \
    usbdevice.cpp \
    dfuhandler.cpp \
    targetmemory.cpp \
    dfusefile.cpp \
    QLogger/QLogger.cpp \
    profiledlg.cpp \
    keywidget.cpp \
    combodelegate.cpp \
    gamecombobox.cpp \
    ../common/ps.cpp \
    listdelegate.cpp \
    buttonsconfig.cpp \
    util.cpp \
    colordlg.cpp \
    clickablelabel.cpp \
    profile.cpp \
    ../common/crc32.cpp \
    keywidgetcontainer.cpp \
    keylinewidget.cpp \
    hidthread.cpp

HEADERS  += dialog.h \
    hidhandler.h \
    usbdevice.h \
    dfuhandler.h \
    targetmemory.h \
    dfusefile.h \
    QLogger/QLogger.h \
    profiledlg.h \
    keywidget.h \
    combodelegate.h \
    gamecombobox.h \
    ../common/ps.h \
    listdelegate.h \
    buttonsconfig.h \
    util.h \
    about.h \
    colordlg.h \
    clickablelabel.h \
    profile.h \
    ../common/crc32.h \
    keywidgetcontainer.h \
    keylinewidget.h \
    hidthread.h \
    ../common/profiledata.h \
    version.h


FORMS    += dialog.ui \
    profiledlg.ui \
    colordlg.ui

RC_ICONS = images/diablo.ico

RESOURCES += \
    BeastBox.qrc

TRANSLATIONS += translations/app.pt_BR.ts

#unix|win32: LIBS += -L$$PWD/../libusb/MinGW32/static/ -lusb-1.0

win32: LIBS += -L$$PWD/../libusb/MinGW32/static/ -lusb-1.0
else:unix:LIBS += -lusb-1.0

INCLUDEPATH += $$PWD/../libusb/include/libusb-1.0
DEPENDPATH += $$PWD/../libusb/include/libusb-1.0

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../libusb/MinGW32/static/libusb-1.0.a
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../libusb/MinGW32/static/libusb-1.0.a
