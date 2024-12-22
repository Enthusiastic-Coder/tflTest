#include "NetworkRailScheduleCSV.h"


void NetworkRailScheduleCSV::onLine(int lineNo, const QStringList &args) {

    if( args.length()>=3)
    {
        _data[args[0]].firstStanox = args[1];
        _data[args[0]].lastStanox = args[2];
    }
}
