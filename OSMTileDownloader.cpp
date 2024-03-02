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

    connect(_networkAccessManager, &QNetworkAccessManager::finished, this, [this](QNetworkReply* reply)
            {
        emit downloadCompleted();

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
                        img.save(getFilename(zoom, x, y));
                    }
                }
                else
                {
                    qDebug() << Q_FUNC_INFO << "-" << reply->errorString();
                }
            });

    connect(this, &OSMTileDownloader::downloadCompleted, this, [this] {

            if( _itemsToDownload.empty())
            {
                _callBack("All Tile downloads completeed.");
                return;
            }

            downloadItem item = _itemsToDownload.back();
            _itemsToDownload.pop_back();
            downloadTile(item.url, item.x, item.y, item.zoom);

            _callBack(QString("Tiles left:%1").arg(static_cast<int>(_itemsToDownload.size())));

        }, Qt::QueuedConnection);

    _userInfo = QCoreApplication::applicationName();
    _userInfo += " ";
    _userInfo += QSysInfo::prettyProductName();
}

void OSMTileDownloader::generate(TileCorners corners, int zoomLevel, std::function<void(QString msg)> callback)
{
    QDir dir;

    dir.mkpath(rootDir);

    for(int zoom=1; zoom <= 16; ++zoom)
        dir.mkpath(rootDir + QDir::separator() + QString::number(zoom));

    _callBack = callback;
    _itemsToDownload.clear();

    const TileCoordinates coordsTopLeft = mapGPSToTile(corners.topLeft.latitude, corners.topLeft.longitude, zoomLevel);
    const TileCoordinates coordsBottomRight = mapGPSToTile(corners.bottomRight.latitude, corners.bottomRight.longitude, zoomLevel);

    bool downloadTriggered = false;
    for(int x = coordsTopLeft.x; x <= coordsBottomRight.x; ++x)
    {
        for(int y = coordsTopLeft.y; y <= coordsBottomRight.y; ++y)
        {
            const QString tileFilename = getFilename(zoomLevel, x, y);

            if( QFile::exists(tileFilename))
                continue;

            const QString url = getTileURL(osmPath, zoomLevel, x, y);

            downloadItem item;
            item.url = url;
            item.x = x;
            item.y = y;
            item.zoom = zoomLevel;

            _itemsToDownload.push_back(item);

            if( !downloadTriggered)
            {
                downloadTriggered = true;
                emit downloadCompleted();
            }
        }
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

QString OSMTileDownloader::getFilename(int zoom, int x, int y) const
{
    return QString("%1/%2/%3_%4.png").arg(rootDir).arg(zoom).arg(x).arg(y);
}

