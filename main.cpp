#include "Widget.h"
#include <QApplication>
#include <QSslSocket>
#include <QMessageBox>
#include <QLibrary>
#include <QDir>

#include <GPSBoundary.h>
#include <QVector>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication a(argc, argv);
//    QLibrary ssl;
//    ssl.setFileName("ssla");
//    QLibrary crypto;
//    crypto.setFileName("cryptoa");

    qDebug() << QApplication::libraryPaths();
    qDebug() << QDir::currentPath();
    qDebug() << QSslSocket::sslLibraryVersionString();
    qDebug() << QSslSocket::sslLibraryBuildVersionString();
    if (!QSslSocket::supportsSsl()) {
            QMessageBox::information(0, "Secure Socket Client",
                                     "This system does not support SSL/TLS.");
//            return -1;
        }

    GPSBoundary screen(GPSLocation(0,0), GPSLocation(1,1));

    QVector<QPair<GPSLocation, GPSLocation>> list;
    list << qMakePair(GPSLocation(0,-2), GPSLocation(1,-1));
    list << qMakePair(GPSLocation(0,-2), GPSLocation(1,1));
    list << qMakePair(GPSLocation(2,0), GPSLocation(1,1));

    for(const auto& item : list)
        qDebug() << QString::fromStdString(item.first.toString()) << "->" << QString::fromStdString(item.second.toString()) << " = " << screen.contains(item);

    Widget w;

#ifdef Q_OS_ANDROID
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
