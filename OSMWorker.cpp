#include "OSMWorker.h"

#include <QElapsedTimer>
#include <QFile>

#include <QDebug>

OSMWorker::OSMWorker(QObject *parent) : QObject(parent)
{

}

void OSMWorker::process(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("File %s not found", file.fileName().toLocal8Bit().data());
        return;
    }

    QElapsedTimer t;
    t.start();

    _allNodes.clear();
    _allWayPoints.clear();

    QTextStream stream(&file);

    qlonglong lineCount(0);
    qlonglong currentWayID = -1;
    qlonglong currentNodeID = -1;
    bool bOnNodes(true);

    //<bounds minlat="51.28554" minlon="-0.511482" maxlat="51.69344" maxlon="0.335437"/>

    while( !stream.atEnd())
    {
        lineCount++;
        QString line = stream.readLine().trimmed();

        if( line.startsWith(QLatin1String("<node")))
        {
            line.remove(QChar('"'));
            line.remove(QChar('\''));
            QStringList sl = line.split(QStringLiteral(" "), QString::SkipEmptyParts);

            NODE node;
            qlonglong nodeId = sl[1].mid(3).toLongLong();
            node.Lat = sl[2].mid(4).toDouble();
            node.Lng = sl[3].mid(4).toDouble();

            _allNodes[nodeId] = node;
            currentNodeID = nodeId;
        }

        if( line.startsWith(QLatin1String("<way")))
        {
            bOnNodes = false;
            line.remove(QChar('"'));
            line.remove(QChar('\''));
            QStringList sl = line.split(" ", QString::SkipEmptyParts);

            currentWayID = sl[1].mid(3).toLongLong();
        }
        else if( line.startsWith(QLatin1String("<nd ref")))
        {
            line.remove(QChar('"'));
            line.remove(QChar('\''));
            QStringList sl = line.split(" ", QString::SkipEmptyParts);

            WAY& way = _allWayPoints[currentWayID];
            way.pts << sl[1].mid(4).toLongLong();
        }
        else if( line.startsWith(QLatin1String("<tag")))
        {
            line.remove(QChar('"'));
            line.remove(QChar('\''));
            QStringList sl = line.split(" ", QString::SkipEmptyParts);
            QString key = sl[1].mid(2);
            QString value = sl[2].mid(2);

            if( bOnNodes)
            {
                NODE& node = _allNodes[currentNodeID];
                node.keyValues[key] = value;
            }
            else
            {
                WAY& way = _allWayPoints[currentWayID];
                way.keyValues[key] = value;
            }
        }
        else if( line.startsWith(QLatin1String("<relation")))
            break;

        if( lineCount % 100000 == 0)
            qDebug() << "Line count : " << lineCount;

    }

    qDebug() << "-------------------------------------";
    qDebug() << "Total Lines : " << lineCount;
    qDebug() << "Nodes Counted : " << _allNodes.size();
    qDebug() << "Waypoints Counted : " << _allWayPoints.size();
    qDebug() << "Seconds : " << t.elapsed()/1000.0;
    qDebug() << "-------------------------------------";
}

void OSMWorker::filter(const QString &key, const QString &value)
{
    _resultOutput.clear();

    for( const auto& wayPoint : _allWayPoints)
    {
        bool bFound(false);

        for(auto it = wayPoint.keyValues.begin(); !bFound && it != wayPoint.keyValues.end(); ++it)
            bFound = it.key() == key && it.value() == value;

        if( !bFound )
            continue;

        std::unique_ptr<WAYPOINT> wp( new WAYPOINT);

        auto itName = wayPoint.keyValues.find(QStringLiteral("Name"));

        if( itName != wayPoint.keyValues.end())
        {
            wp->tagCount = 1;
            QLatin1String str(itName.value().toLatin1());
            wp->word.push_back(str);
            wp->wordCounts.push_back(str.size());
        }

        for(const auto& pt : wayPoint.pts)
        {
            NODE& node = _allNodes[pt];
            wp->pt.push_back(std::make_pair(node.Lat, node.Lng));
        }

        wp->ptsCount = wp->pt.size();
        _resultOutput.push_back(std::move(wp));
    }

    qDebug() << "Filter Count : " << _resultOutput.size();
}
