#include "NetworkRailStnsCSV.h"

NetworkRailStnsCSV::NetworkRailStnsCSV()
{

}


void NetworkRailStnsCSV::onLine(int lineNo, const QStringList& args)
{
    auto& item = _data[args[0]];
    item.location = args[1];
    item.stanme = args[2];
}

const NetworkRailStnsCSV::DATA& NetworkRailStnsCSV::operator[](QString id) const
{
    auto it = _data.find(id);
    if( it == _data.end())
    {
        static DATA empty;
        empty.location = "";
        empty.stanme = "";

        return empty;
    }

    return *it;
}
