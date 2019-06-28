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
    OSMWorker.cpp

HEADERS += \
        Widget.h \
    TFLRouteWorker.h \
    OSMWorker.h

FORMS += \
        Widget.ui

data.files += data/*
data.path = $$DEPLOY_DIR/data
INSTALLS += data

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include(sdk/qMake/qMakeDestination.pri)

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

android {
    WANTSSL10 {
        OPENSSLFOLDER = "1.0"
    } else  {
        OPENSSLFOLDER = "1.1"
    }

    ANDROID_EXTRA_LIBS += \
        $$PWD/openssl/$$TARGET_ARCH_ABI/$$OPENSSLFOLDER/libcrypto.so \
        $$PWD/openssl/$$TARGET_ARCH_ABI/$$OPENSSLFOLDER/libssl.so
}
