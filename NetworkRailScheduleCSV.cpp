#include "NetworkRailScheduleCSV.h"


void NetworkRailScheduleCSV::onLine(int lineNo, const QStringList &parts) {

    NetworkRailScheduleDATA schedule;

    schedule.atocCode = parts[0];
    schedule.serviceCode = parts[1];
    schedule.firstStanox = parts[2];
    schedule.lastStanox = parts[3];
    schedule.firstLocation = parts[4];
    schedule.lastLocation = parts[5];
    schedule.departTime = parts[6];
    schedule.arrivalTime = parts[7];
    schedule.startDate = QDate::fromString(parts[8], "yyyy-MM-dd");
    schedule.endDate = QDate::fromString(parts[9], "yyyy-MM-dd");
    schedule.daysRun = parts[10];

    _data[schedule.atocCode] = schedule;
}
