#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QMap>
#include <QTextBrowser>
#include "TFLRouteWorker.h"
#include "OSMWorker.h"

namespace Ui {
class Widget;
}

struct Vehicle
{
    Vehicle() {}

    QString vehicleId;
    QString stationName;
    QString currentLocation;
    QString towards;
    QString direction;
    int timeToStation = 0;
    QString destinationName;
    QString platformName;
    QString naptanId;
    QString lineId;

    void formatCurrentLocation()
    {
        if( currentLocation.startsWith("Approaching"))
        {
            currentLocation = currentLocation.mid(QString("Approaching").length()+1);
        }
        else if( currentLocation.startsWith("At"))
        {
            int idx = currentLocation.lastIndexOf("Platform");
            currentLocation = currentLocation.mid(3, idx-4);
        }
        else if( currentLocation.startsWith("Between"))
        {
            int idx = currentLocation.lastIndexOf("and");
            currentLocation = currentLocation.mid(8, idx-9);
        }
        else if( currentLocation.startsWith("Left"))
        {
            currentLocation = currentLocation.mid(QString("Left").length()+1);
        }
        else if( currentLocation.startsWith("Departed"))
        {
            currentLocation = currentLocation.mid(QString("Departed").length()+1);
        }
    }

    QString toString() const
    {
        return QString("vehicleID:%1 [%12]\nStn:%2\nPlat:%7\nNaptanId:%3\nCurr:[%4]\nToward:%9\nDir:%8\nTimeTo:%5 [%10m][%11mins]\nDest:%6")
                .arg(vehicleId)
                .arg(stationName)
                .arg(naptanId)
                .arg(currentLocation)
                .arg(timeToStation)
                .arg(destinationName)
                .arg(platformName)
                .arg(direction)
                .arg(towards)
                .arg(timeToStation/100.0/1.609334, 0,'f', 2)
                .arg(int(timeToStation/60.0))
                .arg(lineId);
    }
};

struct LatLng
{
    float fLat = 0.0f;
    float fLng = 0.0f;
};

struct OldStopPoint
{
    QString name;
    QString naptanId;
    LatLng location;

    QString toString() const
    {
        return QString("%1, %2, [%3,%4]")
                .arg(name)
                .arg(naptanId)
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

protected slots:
    void startTFLRoutesDownload();
    void startTFLStopPointDownload();
    void processUSStates();

protected:
    void parseLineArrival(const QJsonDocument& doc, bool bMinimize=true);
    void parseStopPoints(const QJsonDocument& doc);
    void updateTextBrowserWithArrivals(QTextBrowser *textBrowser);
    void updateTextBrowserWithStations(QTextBrowser *textBrowser);


private:
    const QString lineStopPointsURL = "https://api.tfl.gov.uk/line/%1/stoppoints";
    const QString lineArrivalsURL = "https://api.tfl.gov.uk/line/%1/arrivals";
    const QString vehicleArrivalsURL = "https://api.tfl.gov.uk/vehicle/%1/arrivals";

    Ui::Widget *ui;
    QNetworkAccessManager* _manager;
    QMap<QString, Vehicle> _trains;
    QHash<QString,OldStopPoint> _stations;
    TFLRouteWorker* _tflWorker = new TFLRouteWorker(this);
    OSMWorker* _osmWorker = new OSMWorker(this);
};

#endif // WIDGET_H
