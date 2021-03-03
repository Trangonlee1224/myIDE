#-------------------------------------------------
#
# Project created by QtCreator 2019-08-31T12:38:25
#
#-------------------------------------------------

QT       += core gui
#LIBS += -lqscintilla2_qt5
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QscitinallaLearning
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/release/ -lqscintilla2_qt5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/debug/ -lqscintilla2_qt5
else:unix: LIBS += -L$$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/ -lqscintilla2_qt5

INCLUDEPATH += $$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/release
DEPENDPATH += $$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/release

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/release/libqscintilla2_qt5.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/debug/libqscintilla2_qt5.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/release/qscintilla2_qt5.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/debug/qscintilla2_qt5.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../newqt/QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_12_2_MinGW_32_bit-Release/libqscintilla2_qt5.a

RESOURCES += \
    myrecourses.qrc
