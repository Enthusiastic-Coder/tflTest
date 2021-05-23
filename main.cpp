#include "Widget.h"
#include <QApplication>
#include <QSslSocket>
#include <QMessageBox>
#include <QLibrary>
#include <QDir>

#include <QVector>
#include <QDebug>

#include <vector>

void leastSquares()
{
    std::vector<Vector3F> pts;

    for( int i=0; i< 50; ++i)
//        pts.push_back(Vector3F(i,i*0.1f,0));
        pts.push_back(Vector3F(std::rand()%9-4, i,i));

    if(pts.size() <3)
        return;

    Vector3F sum;

    for(Vector3F pt:pts)
    {
        sum += pt;
    }

    Vector3F centroid;

    centroid = sum / float(pts.size());

    float xx(0.0f), xy(0.0f), xz(0.0f);
    float yy(0.0f), yz(0.0f), zz(0.0f);

    for(Vector3F pt:pts)
    {
        Vector3F r = pt-centroid;
        xx += r.x * r.x;
        xy += r.x * r.y;
        xz += r.x * r.z;

        yy += r.y * r.y;
        yz += r.y * r.z;
        zz += r.z * r.z;
    }

    float det_X = yy * zz - yz * yz;
    float det_Y = xx * zz - xz * xz;
    float det_Z = xx * yy - xy * xy;

    float det_max = std::max(det_X, std::max(det_Y, det_Z));

    if( det_max <= 0.0)
        return;

    Vector3F dir;

    if( det_max == det_X)
        dir = Vector3F(det_X, xz*yz - xy*zz, xy*yz - xz*yy);

    else if(det_max == det_Y )
        dir = Vector3F( xz*yz - xy*zz, det_Y, xy*xz - yz*xx);

    else
        dir = Vector3F( xy*yz - xz*yy, xy*xz - yz*xx, det_Z);

    dir.Normalize();
    qDebug() << "DIR :" << QString::fromStdString(dir.toString());
    qDebug() << "CEN :" << QString::fromStdString(centroid.toString());
}

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

    QString str = QString::number(__LINE__);
    QString str2 = Q_FUNC_INFO;
    QString str3 = __FILE__;

    leastSquares();

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
