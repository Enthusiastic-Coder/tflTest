#ifndef NETWORKRAILSCHEDULEJSON_H
#define NETWORKRAILSCHEDULEJSON_H

#include <QString>
#include <QTime>
#include <QDate>
#include <QBitArray>


struct NRScheduleStnDATA
{
    QString location;
    QString stanox;
    QTime arrivalTime;
};

struct NRScheduleDATA {
    QString atocCode;
    QString serviceCode;
    QDate startDate;
    QDate endDate;
    QBitArray daysRun;

    std::vector<NRScheduleStnDATA> stations;
};

class NetworkRailScheduleJSON
{
public:
    void load(const QString& filename);
};

#endif // NETWORKRAILSCHEDULEJSON_H
