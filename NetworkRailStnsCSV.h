#ifndef NETWORKRAILSTNSCSV_H
#define NETWORKRAILSTNSCSV_H

#include "csvfileload.h"
#include <QHash>

class NetworkRailStnsCSV : public CSVFileLoad
{
public:
    struct DATA {
        QString location;
        QString stanme;
    };

    NetworkRailStnsCSV();

    virtual void onLine(int lineNo, const QStringList& args) override;

    const DATA& operator[](QString id) const;

private:
    QHash<QString, DATA> _data;

};

#endif // NETWORKRAILSTNSCSV_H
