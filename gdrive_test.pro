QT       += core gui widgets networkauth

CONFIG += c++11

SOURCES += \
    authprovider.cpp \
    gdriveuploader.cpp \
    main.cpp \
    testwidget.cpp

HEADERS += \
    authprovider.h \
    gdriveuploader.h \
    testwidget.h

FORMS += \
    testwidget.ui

DESTDIR = $$PWD/bin

RESOURCES += \
    resources.qrc
