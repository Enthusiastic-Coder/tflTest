#include "NetworkRailServicesCSV.h"

NetworkRailServicesCSV::NetworkRailServicesCSV()
{

}

void NetworkRailServicesCSV::onLine(int lineNo, const QStringList &args)
{
    if( args.length()==3)
    {
        auto& item = _data[args[2]];
        item.from = args[0];
        item.to = args[1];
    }
    else
    {
        auto& item = _data[args[3]];
        item.from = args[0];
        if( args[2] == QStringLiteral("Direct"))
            item.to = args[1];
        else
            item.to = args[2];
    }
}

const NetworkRailServicesCSV::DATA &NetworkRailServicesCSV::operator[](QString id) const
{
    auto it = _data.find(id);
    if( it == _data.end())
    {
        static DATA empty;
        empty.from = "";
        empty.to = "";

        return empty;
    }

    return *it;
}
