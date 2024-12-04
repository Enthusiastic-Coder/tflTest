#include "OSMData.h"
#include <QFile>
#include <QDebug>
#include <QDataStream>


OSMData::OSMData()
{
}

void OSMData::import(const QString &filename, WAYPOINTS &wayPoints, bool bAllowPoints)
{
    QFile input(filename);
    input.open(QIODevice::ReadOnly);

    if(!input.isOpen())
    {
        qDebug() << input.fileName() << ": OSM FILE NOT FOUND!";
        return;
    }

    QDataStream stream(&input);

    quint64 count;
    stream >> count;

    for( quint64 i = 0; i < count; ++i)
    {
        auto wp = std::make_unique<OSM_WAYPOINT>();

        quint64 tagCount;
        stream >> tagCount;

        wp->tags.resize(tagCount);

        for(quint64 i = 0; i < tagCount; ++i)
        {
            qsizetype len;
            stream >> len;
            QByteArray buffer(len, Qt::Uninitialized);
            stream >> buffer;
            wp->tags[i].first = len;
            wp->tags[i].second = buffer;
        }

        quint64 ptsCount;
        stream >> ptsCount;

        wp->gpsPts.resize(ptsCount);
        if( ptsCount > 1)
        {
            wp->bearings.resize(ptsCount-1);
        }

        for( quint64 i = 0; i < ptsCount; ++i)
            stream >> wp->gpsPts[i]._lat >> wp->gpsPts[i]._lng;

        for( quint64 i = 0; i < ptsCount-1; ++i)
            stream >> wp->bearings[i];

        if( ptsCount > 1 || bAllowPoints)
            wayPoints.push_back(std::move(wp));
    }
}
