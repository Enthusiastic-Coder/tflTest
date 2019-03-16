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
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

include(sdk/qMake/qMakeDestination.pri)

INCLUDEPATH += sdk/include

SOURCES += \
        main.cpp \
        Widget.cpp \
    TFLRouteWorker.cpp \
    GPSBoundary.cpp

HEADERS += \
        Widget.h \
    TFLRouteWorker.h \
    GPSBoundary.h

FORMS += \
        Widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include(sdk/qMake/qMakeDestination.pri)

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/openssl/armeabi-v7a/libcrypto.so \
        $$PWD/openssl/armeabi-v7a/libssl.so
}
 contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/openssl/x86/libcrypto.so \
        $$PWD/openssl/x86/libssl.so
}
