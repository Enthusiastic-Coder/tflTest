#ifndef OSMTILEDOWNLOADER_H
#define OSMTILEDOWNLOADER_H

#include "TileHelpers.h"

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>

class OSMTileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit OSMTileDownloader(QObject *parent = nullptr);

    void generate(TileCorners corners, int zoomLevel, std::function<void (QString)> callback);

signals:

private:
    void downloadTile(const QString& finalURL, int x, int y , int zoom);;

private:
    QNetworkAccessManager* _networkAccessManager = nullptr;
    QNetworkCookieJar cookieJar;
    QString _userInfo;
};

#endif // OSMTILEDOWNLOADER_H
