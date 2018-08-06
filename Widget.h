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
    double timeToStation;
};


class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void parseData(const QJsonDocument& doc);

private:
    Ui::Widget *ui;
    QNetworkAccessManager* _manager;
    QMap<QString, Vehicle> _trains;
};

#endif // WIDGET_H
