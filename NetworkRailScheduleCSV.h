#ifndef NETWORKRAILSCHEDULECSV_H
#define NETWORKRAILSCHEDULECSV_H

#include "csvfileload.h"


struct NetworkRailScheduleDATA {
    QString firstStanox;
    QString lastStanox;
};


class NetworkRailScheduleCSV : public CSVFileLoad<NetworkRailScheduleDATA>
{
public:
    void onLine(int lineNo, const QStringList &args) override;
};

#endif // NETWORKRAILSCHEDULECSV_H
