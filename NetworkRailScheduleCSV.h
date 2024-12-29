#ifndef NETWORKRAILSCHEDULECSV_H
#define NETWORKRAILSCHEDULECSV_H

#include "csvfileload.h"

#include <QDate>
#include <QBitArray>

struct NetworkRailScheduleDATA {
    QString atocCode;
    QString serviceCode;
    QString firstStanox;
    QString lastStanox;
    QString firstLocation;
    QString lastLocation;
    QTime departTime;
    QTime arrivalTime;
    QDate startDate;
    QDate endDate;
    QBitArray daysRun;
};

class NetworkRailScheduleCSV : public CSVFileLoad<NetworkRailScheduleDATA, QMultiHash<QString, NetworkRailScheduleDATA>>
{
public:
    void onLine(int lineNo, const QStringList &parts) override;

    QList<NetworkRailScheduleDATA> getServices(const QString& atoccode, const QString& serviceCode) const;
};

#endif // NETWORKRAILSCHEDULECSV_H
