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

    enum CurrentBehaviour
    {
        Unknown,
        At,
        Approaching,
        Between,
        Left,
        Departed
    };

    Vehicle() {}

    QString vehicleId;
    QString stationName;
    QString originalLocation;
    QString currentLocation;
    QString towards;
    QString _modeName;
    QString direction;
    int timeToStation = 0;
    QString destinationName;
    QString platformName;
    QString naptanId;
    QString lineId;
    CurrentBehaviour _behaviour = Unknown;

    bool isBus() const
    {
        return _modeName == QStringLiteral("bus");
    }

    bool isDLR() const
    {
        return _modeName == QStringLiteral("dlr");
    }

    bool isTube() const
    {
        return _modeName == QStringLiteral("tube");
    }


    void formatCurrentLocation()
    {
        if( currentLocation.isEmpty())
        {
            _behaviour = Unknown;
            return;
        }

        if( currentLocation.startsWith("Approaching"))
        {
            currentLocation = currentLocation.mid(QString("Approaching").length()+1);
            _behaviour = Approaching;
        }
        else if( currentLocation.startsWith(QLatin1String("At")))
        {
            currentLocation = currentLocation.mid(3);
            _behaviour = At;
        }
        else if( currentLocation.startsWith(QLatin1String("Left")))
        {
            currentLocation = currentLocation.mid(QString("Left").length()+1);
            _behaviour = Left;
        }
        else if( currentLocation.startsWith(QLatin1String("Leaving")))
        {
            currentLocation = currentLocation.mid(QString("Leaving").length()+1);
            _behaviour = Left;
        }
        else if( currentLocation.startsWith(QLatin1String("Departed")))
        {
            currentLocation = currentLocation.mid(QString("Departed").length()+1);
            _behaviour = Departed;
        }
        else if( currentLocation.startsWith(QLatin1String("Between")))
        {
            int idx = currentLocation.indexOf(QLatin1String(" and"));
            currentLocation = currentLocation.mid(idx+5);
            _behaviour = Between;
        }
        else if( currentLocation.startsWith(QLatin1String("In between")))
        {
            int idx = currentLocation.indexOf(QLatin1String(" and"));
            currentLocation = currentLocation.mid(idx+5);
            _behaviour = Between;
        }
        else if( currentLocation.startsWith(QLatin1String("Near")))
        {
            currentLocation = currentLocation.mid(QString("Near").length()+1);
            _behaviour = Approaching;
        }

        {
            int idx = currentLocation.lastIndexOf(QLatin1String(" Platform"));
            if( idx != -1)
                currentLocation = currentLocation.left(idx);
        }

        currentLocation = currentLocation.trimmed();

        if( currentLocation.startsWith(QStringLiteral("Kings Cross")))
            currentLocation = QStringLiteral("King's Cross");

        else if( currentLocation == QStringLiteral("Castle and Kennington"))
            currentLocation = QStringLiteral("Elephant & Castle");

        else if( currentLocation == QStringLiteral("Elephant and Castle"))
            currentLocation = QStringLiteral("Elephant & Castle");

        else if( currentLocation.startsWith(QStringLiteral("Earls Court")))
            currentLocation = QStringLiteral("Earl's Court");

        else if( currentLocation.startsWith(QStringLiteral("Paddington (Suburban)")))
            currentLocation = QStringLiteral("Paddington");

        else if( currentLocation.startsWith(QStringLiteral("Heathrow Terminal 1")))
            currentLocation = QStringLiteral("Heathrow Terminals 2");

        else if( currentLocation.startsWith(QStringLiteral("Chalfont and Latimer")))
            currentLocation = QStringLiteral("Chalfont & Latimer");

        else if( currentLocation.startsWith(QStringLiteral("Shepherds Bush")))
            currentLocation = QStringLiteral("Shepherd's Bush");

        else if( currentLocation.startsWith(QStringLiteral("Regents Park")))
            currentLocation = QStringLiteral("Regent's Park");

        else if( currentLocation.startsWith(QStringLiteral("Angel")))
            currentLocation = QStringLiteral("Angel");

        else if( currentLocation.startsWith(QStringLiteral("White City")))
            currentLocation = QStringLiteral("White City");

        else if( currentLocation.startsWith(QStringLiteral("London Bridge")))
            currentLocation = QStringLiteral("London Bridge");

        else if( currentLocation.startsWith(QStringLiteral("East Finchley")))
            currentLocation = QStringLiteral("East Finchley");

        else if( currentLocation.startsWith(QStringLiteral("Turnham Green")))
            currentLocation = QStringLiteral("Turnham Green");

        else if( currentLocation.startsWith(QStringLiteral("Heathrow T2")))
            currentLocation = QStringLiteral("Heathrow Terminals 2");

        else if( currentLocation.startsWith(QStringLiteral("Camden Town")))
            currentLocation = QStringLiteral("Camden Town");

        else if( currentLocation == QStringLiteral("Upminster") && isTube())
            currentLocation += QStringLiteral(" Under");
    }

    QString toString() const
    {
        return QString("vehicleID:%1 [%12]\nStn:%2\nPlat:%7\nNaptanId:%3\nOrigCurr:[%13]\nCurr:[%4]\nToward:%9\nDir:%8\nTimeTo:%5 [%10m][%11mins]\nDest:%6")
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
                .arg(lineId)
                .arg(originalLocation);
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
