#ifndef NETWORKRAILSTNSCSV_H
#define NETWORKRAILSTNSCSV_H

#include "csvfileload.h"

struct NetworkRailStnsDATA {
    QString location;
    QString stanme;
};

class NetworkRailStnsCSV : public CSVFileLoad<NetworkRailStnsDATA>
{
public:

    virtual void onLine(int lineNo, const QStringList& args) override;

};

#endif // NETWORKRAILSTNSCSV_H
