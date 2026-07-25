#-------------------------------------------------
#
# Project created by QtCreator 2016-11-28T18:58:07
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = buckshot
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += main.cpp\
        mainwindow.cpp\
        modedescriptor.cpp

HEADERS  += mainwindow.h\
        modedescriptor.h

FORMS    += mainwindow.ui

#@todo: integrate
#DISTFILES += \
#    ../b2d

RESOURCES += \
    assets.qrc

# For issues of launching from Ubuntu desktops
linux: QMAKE_LFLAGS += -no-pie

# Qt's qyieldcpu.h calls __yield() without declaring it, which Apple Clang 17+
# rejects (QTBUG-135402); arm_acle.h provides the declaration. Drop this once
# Qt ships the reordered __builtin_arm_yield check.
macx:contains(QMAKE_HOST.arch, arm64): QMAKE_CXXFLAGS += -include arm_acle.h
