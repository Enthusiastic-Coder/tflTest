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
    w.show();

    return a.exec();
}
