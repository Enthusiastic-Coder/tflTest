#include "Widget.h"
#include <QApplication>
#include <QSslSocket>
#include <QMessageBox>
#include <QLibrary>
#include <QDir>

#include <QVector>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache);
//    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication a(argc, argv);
//    QLibrary ssl;
//    ssl.setFileName("ssla");
//    QLibrary crypto;
//    crypto.setFileName("cryptoa");

    float fAlt = 17200;

    int tAlt = fAlt/100;
    int roundedAlt = qRound(fAlt/1000.0f);
    roundedAlt *= 10;

    QString strTAlt;
    if( std::abs(roundedAlt - tAlt) < 2 )
        strTAlt = QString("FL%0").arg(roundedAlt);
    else
        strTAlt = QString("FL%0").arg(tAlt);


    a.setOrganizationName("com.enthusiasticcoder.tfltest");
    a.setOrganizationDomain("enthusiasticcoder");
    a.setApplicationName("TFLTest");

    qDebug() << QApplication::libraryPaths();
    qDebug() << QDir::currentPath();
    qDebug() << QSslSocket::sslLibraryVersionString();
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    if (!QSslSocket::supportsSsl()) {
            QMessageBox::information(0, "Secure Socket Client",
                                     "This system does not support SSL/TLS.");
//            return -1;
        }

    Widget w;

#ifdef Q_OS_ANDROID
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
