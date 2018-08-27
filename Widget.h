#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QMap>
#include <QTextBrowser>

namespace Ui {
class Widget;
}

struct Vehicle
{
    Vehicle() {}

    QString id;
    QString vehicleId;
    QString stationName;
    QString currentLocation;
    QString direction;
    int timeToStation = 0;
    QString destinationName;
    QString platformName;

    QString toString() const
    {
        return QString("id:%1, %7\nStn:%2\nCurr:%3\nDir:%8\nTimeTo:%4\nDest:%5\nPlat:%6")
                .arg(id)
                .arg(stationName)
                .arg(currentLocation)
                .arg(timeToStation)
                .arg(destinationName)
                .arg(platformName)
                .arg(vehicleId)
                .arg(direction);
    }
};

struct LatLng
{
    float fLat = 0.0f;
    float fLng = 0.0f;
};

struct StopPoint
{
    QString name;
    LatLng location;

    QString toString() const
    {
        return QString("%1, [%2,%3]")
                .arg(name)
                .arg(location.fLat)
                .arg(location.fLng);
    }
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void parseLineArrival(const QJsonDocument& doc);
    void parseStopPoints(const QJsonDocument& doc);
    void updateTextBrowserWithArrivals(QTextBrowser *textBrowser);
    void updateTextBrowserWithStations(QTextBrowser *textBrowser);

private:
    Ui::Widget *ui;
    QNetworkAccessManager* _manager;
    QMap<QString, Vehicle> _trains;
    QMap<QString,StopPoint> _stations;
};

#endif // WIDGET_H
