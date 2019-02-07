#include "Widget.h"
#include <QApplication>
#include <QSslSocket>
#include <QMessageBox>
#include <QLibrary>
#include <QDir>

int main(int argc, char *argv[])
{
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

    Widget w;

#ifdef Q_OS_ANDROID
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
