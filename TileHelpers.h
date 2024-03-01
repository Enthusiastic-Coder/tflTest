#pragma once

#include <QString>
#include <QtMath>

struct GPSCoordinates {
    double latitude;
    double longitude;
};

struct TileCoordinates {
    int x;
    int y;
};

struct TileCorners {
    GPSCoordinates topLeft;
    GPSCoordinates bottomRight;
};

static TileCorners calculateTileCorners(int tileX, int tileY, int zoom) {
    // Calculate GPS coordinates of the top-left corner of the current tile
    double lon_deg_top_left = tileX / pow(2, zoom) * 360.0 - 180.0;
    double lat_rad_top_left = atan(sinh(M_PI * (1 - 2 * tileY / pow(2, zoom))));
    double lat_deg_top_left = lat_rad_top_left * 180.0 / M_PI;

    // Calculate GPS coordinates of the bottom-right corner of the current tile
    double lon_deg_bottom_right = (tileX + 1) / pow(2, zoom) * 360.0 - 180.0;
    double lat_rad_bottom_right = atan(sinh(M_PI * (1 - 2 * (tileY + 1) / pow(2, zoom))));
    double lat_deg_bottom_right = lat_rad_bottom_right * 180.0 / M_PI;

    TileCorners corners;
    corners.topLeft = {lat_deg_top_left, lon_deg_top_left};
    corners.bottomRight = {lat_deg_bottom_right, lon_deg_bottom_right};

    return corners;
}

static TileCoordinates mapGPSToTile(double latitude, double longitude, int zoom) {
    int n = pow(2, zoom);
    double xtile = n * ((longitude + 180) / 360);
    double ytile = n * (1 - (log(tan(latitude * M_PI / 180) + 1 / cos(latitude * M_PI / 180)) / M_PI)) / 2;

    return {static_cast<int>(xtile), static_cast<int>(ytile)};
}

static QString getTileURL(QString url, int zoom, int x, int y) {
    url = url.replace("{x}", QString::number(x));
    url = url.replace("{y}", QString::number(y));
    url = url.replace("{zoom}", QString::number(zoom));
    return url;
}

static QString getTileId(int zoom, int tileX, int tileY) {
    return QString("%1/%2/%3").arg(zoom).arg(tileX).arg(tileY);
}
