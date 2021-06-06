#ifndef NETWORKRAILSERVICESCSV_H
#define NETWORKRAILSERVICESCSV_H

#include "csvfileload.h"
#include <QHash>

class NetworkRailServicesCSV : public CSVFileLoad
{
public:
    struct DATA {
        QString from;
        QString to;
    };

    NetworkRailServicesCSV();

    virtual void onLine(int lineNo, const QStringList& args) override;

    const DATA& operator[](QString id) const;
private:
    QHash<QString, DATA> _data;
};

#endif // NETWORKRAILSERVICESCSV_H
