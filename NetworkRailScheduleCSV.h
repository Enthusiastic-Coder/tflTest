#ifndef NETWORKRAILSCHEDULECSV_H
#define NETWORKRAILSCHEDULECSV_H

#include "csvfileload.h"

#include <QDate>

struct NetworkRailScheduleDATA {
    QString atocCode;
    QString serviceCode;
    QString firstStanox;
    QString lastStanox;
    QString firstLocation;
    QString lastLocation;
    QString departTime;
    QString arrivalTime;
    QDate startDate;
    QDate endDate;
    QString daysRun;
};

class NetworkRailScheduleCSV : public CSVFileLoad<NetworkRailScheduleDATA>
{
public:
    void onLine(int lineNo, const QStringList &parts) override;
};

#endif // NETWORKRAILSCHEDULECSV_H
