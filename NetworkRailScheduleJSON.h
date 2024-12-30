#ifndef NETWORKRAILSCHEDULEJSON_H
#define NETWORKRAILSCHEDULEJSON_H

#include <QString>
#include <QTime>
#include <QDate>
#include <QBitArray>
#include <QList>
#include <QMultiHash>
#include <QHash>

struct NRScheduleDATA {
    QString atocCode;
    QString serviceCode;
    QDate startDate;
    QDate endDate;
    QBitArray daysRun;
    QString destinationStanox;

    QHash<QString, QTime> stations;
};

class NetworkRailScheduleJSON
{
public:
    void load(const QString& filename);

    QString getDestination(const QString& atoccode, const QString& serviceCode, const QString& stanox, const QTime& now) const;

private:
    QMultiHash<QString,NRScheduleDATA> _services;
};

#endif // NETWORKRAILSCHEDULEJSON_H
