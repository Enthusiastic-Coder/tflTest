#ifndef NETWORKRAILSERVICESCSV_H
#define NETWORKRAILSERVICESCSV_H

#include "csvfileload.h"

struct NetworkRailServicesDATA {
    QString from;
    QString to;
};

class NetworkRailServicesCSV : public CSVFileLoad<NetworkRailServicesDATA>
{
public:

    virtual void onLine(int lineNo, const QStringList& args) override;
};

#endif // NETWORKRAILSERVICESCSV_H
