#include "OSMData.h"
#include <QFile>
#include <QDebug>
#include <QDataStream>

#define ENTRY_WP(name) name.setBounds(GPSLocation(51.69344,-0.511482), GPSLocation(51.28554,0.335437), 40)
#define VB_WP(name,limit) name.setViewBoundary(limit.first, limit.second)

OSMData::OSMData()
{
    ENTRY_WP(_osmMotorway);
    ENTRY_WP(_osmPrimary);
    ENTRY_WP(_osmSecondary);
    ENTRY_WP(_osmTertiary);
    ENTRY_WP(_osmResidential);
    ENTRY_WP(_osmFootway);
    ENTRY_WP(_osmWater);
    ENTRY_WP(_osmAeroway);
    ENTRY_WP(_osmAeroRunway);
    ENTRY_WP(_osmCycleway);
    ENTRY_WP(_osmPedestrian);
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
        WAYPOINT* wp(new WAYPOINT);

        quint64 tagCount;
        stream >> tagCount;

        wp->tags.resize(tagCount);

        for(quint64 i = 0; i < tagCount; ++i)
        {
            int len;
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
            wp->distances.resize(ptsCount-1);
        }

        for( quint64 i = 0; i < ptsCount; ++i)
            stream >> wp->gpsPts[i]._lat >> wp->gpsPts[i]._lng;

        for( quint64 i = 0; i < ptsCount-1; ++i)
            stream >> wp->distances[i];

        for( quint64 i = 0; i < ptsCount-1; ++i)
            stream >> wp->bearings[i];

        if( ptsCount > 1 || bAllowPoints)
            wayPoints.add(wp, wp->gpsPts[0]);
    }
}
