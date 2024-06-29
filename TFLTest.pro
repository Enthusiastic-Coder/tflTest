#-------------------------------------------------
#
# Project created by QtCreator 2018-08-05T17:29:02
#
#-------------------------------------------------

QT       += core gui widgets network

TARGET = TFLTest
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

######################################################
### PUT into separate

CONFIG(release, debug|release) {
    CONFIGURATION = "Release"
}

CONFIG(debug, debug|release) {
    CONFIGURATION = "Debug"
}

win32{
DESTDIR = $$OUT_PWD/$$CONFIGURATION
OBJECTS_DIR = $$OUT_PWD/.obj
MOC_DIR = $$OUT_PWD/.moc
RCC_DIR = $$OUT_PWD/.rcc
UI_DIR = $$OUT_PWD/.ui


    DEPLOY_DIR = $$DESTDIR
}

android{
    DEPLOY_DIR = /assets
}

isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
}
######################################################

include(qtstomp/qtstomp.pri)

INCLUDEPATH += sdk/include

SOURCES += \
    NetworkRailServicesCSV.cpp \
    NetworkRailStnsCSV.cpp \
    OSMRendererBase.cpp \
    OSMTileDownloader.cpp \
    OSMTileGenerator.cpp \
    OSMRenderer.cpp \
    RawScaleOSMRenderer.cpp \
    TFLRouteCompression.cpp \
    csvfileload.cpp \
        main.cpp \
        Widget.cpp \
    TFLRouteWorker.cpp \
    OSMWorker.cpp \
    OSMData.cpp

HEADERS += \
    NetworkRailServicesCSV.h \
    NetworkRailStnsCSV.h \
    OSMRendererBase.h \
    OSMTileDownloader.h \
    OSMTileGenerator.h \
    OSMRenderer.h \
    RawScaleOSMRenderer.h \
    TFLRouteCompression.h \
    TileHelpers.h \
        Widget.h \
    TFLRouteWorker.h \
    OSMWorker.h \
    csvfileload.h \
    OSMData.h

FORMS += \
        Widget.ui

data.files += data/*
data.path = $$DEPLOY_DIR/data
INSTALLS += data

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32 {
openssl.files += openssl/win64/*
openssl.path = $$DEPLOY_DIR
INSTALLS += openssl
}

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

android: include(android_openssl/openssl.pri)
