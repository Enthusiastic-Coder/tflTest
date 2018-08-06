#include "Widget.h"
#include <QApplication>
#include <QSslSocket>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
