#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QMap>

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


class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void parseData(const QJsonDocument& doc);
    void updateTextBrowserWithMap();

private:
    Ui::Widget *ui;
    QNetworkAccessManager* _manager;
    QMap<QString, Vehicle> _trains;
};

#endif // WIDGET_H
