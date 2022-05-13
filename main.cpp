#include "Widget.h"
#include <QApplication>
#include <QSslSocket>
#include <QMessageBox>
#include <QLibrary>
#include <QDir>

#include <QVector>
#include <QDebug>

#include <vector>

#include <MathSupport.h>
#include <Matrix4x4.h>

void leastSquares()
{
    std::vector<Vector3F> pts;

    for( int i=0; i< 50; ++i)
//        pts.push_back(Vector3F(i,i*0.1f,0));
        pts.push_back(Vector3F(std::rand()%9-4, i,i));

    if(pts.size() <3)
        return;

    Vector3F sum = std::accumulate(pts.begin(), pts.end(), Vector3F());

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

////////////////////////////////////////////////////////////////////////////////

    float web[16] = {0.00329677,0.000064086,-0.00120385,0,
    0.0000107041,-0.00350673,-0.000157364,0,
    -0.00120551,0.000144121,-0.00329363,0,
    -0.454338,1.00333,-1.14293,1};

    float niva[16] = {0.00465859, -0.00181589, -6.63375e-06, 0,
    5.26072e-06, -4.76967e-06, 0.00499999, 0,
    -0.00181589, -0.0046586, -2.53341e-06, 0,
    -0.175149, -0.376875, -0.524053, 1};

    /// ? x Web = Niva.
    /// ? = Niva x Web-1;
    ///


    Matrix4x4F webM(web);

    Matrix4x4F nivaM(niva);

    Matrix4x4F result = nivaM * webM.Inverse();

////////////////////////////////////////////////////////////////////////////////

    float nivaMotion[16] = {0.934255,0.0945279,0.343848,0,
                            0.00787045,-0.969458,0.245131,0,
                            0.356518,-0.226309,-0.906465,0
                            ,-0.363465,0.092611,-2.94689,1};


    float webMotion[16] ={0.934255, 0.0945279, 0.343848, 0,
                          0.356518, -0.226309, -0.906465, 0,
                          -0.00787045, 0.969458, -0.245131, 0,
                          -0.363465, 0.0926109, -2.94689, 1};


    Matrix4x4F webMmotion(webMotion);

    Matrix4x4F nivaMmotion(nivaMotion);

    Matrix4x4F resultMotion = nivaMmotion * webMmotion.Inverse();


////////////////////////////////////////////////////////////////////////////////
///
    float webpers[16] = {1.36179,0,0,0,0,2.42095,0,0,0,0,0,-1,1,1,-1,0};

    float nivapers[16] ={1.36179,0,0,0,0,2.42095,0,0,0,0,0,-1,0,0,-1,0};

    Matrix4x4F webMpers(webpers);

    Matrix4x4F nivaMpers(nivapers);

    Matrix4x4F resultPers = nivaMpers * webMpers.Inverse();
////////////////////////////////////////////////////////////////////////////////

    Matrix4x4F modeViewNiva = nivaMpers * nivaMmotion * nivaM;

    Matrix4x4F modelViewWeb = webMpers * webMmotion * webM;

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
