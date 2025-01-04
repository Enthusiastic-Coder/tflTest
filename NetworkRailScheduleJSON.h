#ifndef NETWORKRAILSCHEDULEJSON_H
#define NETWORKRAILSCHEDULEJSON_H

#include <QString>
#include <QTime>
#include <QDate>
#include <QBitArray>
#include <QList>
#include <QMultiHash>
#include <QHash>

struct NRScheduleTimesDATA
{
    QTime arrival;
    QTime departure;
    QTime pass;
};

struct NRScheduleDATA {
    QString toc_id;
    QString serviceCode;
    QDate startDate;
    QDate endDate;
    QBitArray daysRun;
    QString destinationStanox;

    QHash<QString, NRScheduleTimesDATA> stations;
};

class NetworkRailScheduleJSON
{
public:
    void load(const QString& filename);

    void loadFromJson(const QJsonDocument &doc);

    std::tuple<QString, int> getDestination(const QString& toc_id, const QString& serviceCode, const QString& stanox, const QString &eventType, const QTime& now) const;

private:
    QMultiHash<QString,NRScheduleDATA> _services;
};

#endif // NETWORKRAILSCHEDULEJSON_H
