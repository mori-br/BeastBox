#-------------------------------------------------
#
# Project created by QtCreator 2014-12-13T01:09:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gde
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    gamedef.cpp \
    buttondescwidget.cpp \
    ../common/ps.cpp \
    listdelegate.cpp \
    ../common/crc32.cpp

HEADERS  += dialog.h \
    gamedef.h \
    buttondescwidget.h \
    ../common/ps.h \
    listdelegate.h \
    ../common/crc32.h

FORMS    += dialog.ui \
    gamedef.ui

RC_ICONS = images/diablo.ico

RESOURCES += \
    gde.qrc

TRANSLATIONS += translations/app.pt_BR.ts
