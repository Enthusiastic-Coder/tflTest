#include "OSMTileDownloader.h"

#include <QNetworkReply>
#include <QImage>
#include <QCoreApplication>
#include <QDir>

namespace {
const QString rootDir = "osm tile output";
const QString osmPath = "https://tile.openstreetmap.org/{zoom}/{x}/{y}.png";
}

OSMTileDownloader::OSMTileDownloader(QObject *parent)
    : QObject{parent}
{

    _networkAccessManager = new QNetworkAccessManager(this);
    _networkAccessManager->setCookieJar(&cookieJar);

    connect(_networkAccessManager, &QNetworkAccessManager::finished, this, [](QNetworkReply* reply)
            {
                reply->deleteLater();

                QNetworkReply::NetworkError nError = reply->error();

                QNetworkRequest request = reply->request();

                const int x = request.attribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User+0)).toInt();
                const int y = request.attribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User+1)).toInt();
                const int zoom = request.attribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User+2)).toInt();

                if( nError == QNetworkReply::NoError)
                {
                    const QImage img = QImage::fromData(reply->readAll());

                    if( img.isNull())
                    {
                        qDebug() << Q_FUNC_INFO << "-" << reply->errorString();
                    }
                    else
                    {
                        // Save image based on zoom/x/y
                        const QString filename = QString("%1/%2/%3_%4.png").arg(rootDir).arg(zoom).arg(x).arg(y);

                        img.save(filename);
                    }
                }
                else
                {
                    qDebug() << Q_FUNC_INFO << "-" << reply->errorString();
                }
            });

    _userInfo = QCoreApplication::applicationName();
    _userInfo += " ";
    _userInfo += QSysInfo::prettyProductName();
}

void OSMTileDownloader::generate(TileCorners corners, int zoomLevel, std::function<void(QString msg)> callback)
{
    QDir dir;

    dir.mkpath(rootDir);

    for(int zoom=1; zoom <= 14; ++zoom)
        dir.mkpath(rootDir + QDir::separator() + QString::number(zoom));

    const TileCoordinates coordsTopLeft = mapGPSToTile(corners.topLeft.latitude, corners.topLeft.longitude, zoomLevel);
    const TileCoordinates coordsBottomRight = mapGPSToTile(corners.bottomRight.latitude, corners.bottomRight.longitude, zoomLevel);

    for(int x = coordsTopLeft.x; x <= coordsBottomRight.x; ++x)
    {
        for(int y = coordsTopLeft.y; y <= coordsBottomRight.y; ++y)
        {
            const QString url = getTileURL(osmPath, zoomLevel, x, y);
            downloadTile(url, x, y, zoomLevel);
        }

        callback(QString("Currently on %1 of %2...").arg(x, coordsBottomRight.x));
    }
}

void OSMTileDownloader::downloadTile(const QString &finalURL, int x, int y, int zoom)
{
    QUrl url(finalURL);

    QNetworkRequest request(url);

    // Set a user-agent header
    request.setHeader(QNetworkRequest::UserAgentHeader, _userInfo);

    request.setAttribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User+0), x);
    request.setAttribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User+1), y);
    request.setAttribute(static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User+2), zoom);

    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);

    _networkAccessManager->get(request);
}

