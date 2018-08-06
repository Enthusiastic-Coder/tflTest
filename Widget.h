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

    QString vehicleId;
    QString stationName;
    QString currentLocation;
    int timeToStation = 0;
    QString destinationName;

    QString toString() const
    {
        return QString("id:%1\nStn:%2\nCurr:%3\nTimeTo:%4\nDest:%5")
                .arg(vehicleId)
                .arg(stationName)
                .arg(currentLocation)
                .arg(timeToStation)
                .arg(destinationName);
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
