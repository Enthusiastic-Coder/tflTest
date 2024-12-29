#include "NetworkRailScheduleCSV.h"


void NetworkRailScheduleCSV::onLine(int lineNo, const QStringList &parts) {

    NetworkRailScheduleDATA schedule;

    schedule.startDate = QDate::fromString(parts[8], "yyyy-MM-dd");
    schedule.endDate = QDate::fromString(parts[9], "yyyy-MM-dd");

    QDate today = QDate::currentDate();
    if( today < schedule.startDate || today > schedule.endDate) {
        return;
    }

    const QString bitString = parts[10];

    QBitArray bitArray(7);

    for (int i = 0; i < 7; ++i) {
        bitArray.setBit(i, bitString.at(i) == '1');
    }

    if( !bitArray.testBit(today.dayOfWeek()-1)) {
        return;
    }

    schedule.atocCode = parts[0];
    schedule.serviceCode = parts[1];
    schedule.firstStanox = parts[2];
    schedule.lastStanox = parts[3];
    schedule.firstLocation = parts[4];
    schedule.lastLocation = parts[5];
    schedule.departTime = QTime::fromString(parts[6], "hhmm");
    schedule.arrivalTime = QTime::fromString(parts[7], "hhmm");
    schedule.daysRun = bitArray;

    _data.insert( schedule.atocCode + "|" + schedule.serviceCode, schedule);
}

QList<NetworkRailScheduleDATA> NetworkRailScheduleCSV::values(QString key)
{
    return _data.values(key);
}
