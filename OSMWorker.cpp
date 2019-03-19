#include "OSMWorker.h"

#include <QElapsedTimer>
#include <QFile>
#include <QDataStream>
#include <QDebug>

OSMWorker::OSMWorker(QObject *parent) : QObject(parent)
{

}

qlonglong OSMWorker::process(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("File %s not found", file.fileName().toLocal8Bit().data());
        return 0;
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

        line.remove(QChar('"'));
        line.remove(QChar('\''));
        line.remove("/>");
        line.remove(">");

        if( line.startsWith(QLatin1String("<node")))
        {
            QStringList sl = line.split(QStringLiteral(" "), QString::SkipEmptyParts);

            NODE node;
            qlonglong nodeId=0;

            int itemsObtained(0);

            for(const QString& item : sl)
            {
                if( item.startsWith(QStringLiteral("id=")))
                {
                    nodeId = item.mid(3).toLongLong();
                    itemsObtained++;
                }
                if( item.startsWith(QStringLiteral("lat=")))
                {
                    node.Lat = item.mid(4).toDouble();

                    if( node.Lat == 0)
                    {
                        qDebug() << "Lat=0 : " << item;
                    }
                    itemsObtained++;
                }
                if( item.startsWith(QStringLiteral("lon=")))
                {
                    node.Lng = item.mid(4).toDouble();

                    if( node.Lng == 0)
                    {
                        qDebug() << "Lng=0 : " << item;
                    }
                    itemsObtained++;
                }
                if( itemsObtained == 3)
                    break;
            }

            _allNodes[nodeId] = node;
            currentNodeID = nodeId;
        }

        if( line.startsWith(QLatin1String("<way")))
        {
            bOnNodes = false;
            QStringList sl = line.split(" ", QString::SkipEmptyParts);

            currentWayID = sl[1].mid(3).toLongLong();
        }
        else if( line.startsWith(QLatin1String("<nd ref")))
        {
            QStringList sl = line.split(" ", QString::SkipEmptyParts);

            WAY& way = _allWayPoints[currentWayID];
            way.pts << sl[1].mid(4).toLongLong();
        }
        else if( line.startsWith(QLatin1String("<tag")))
        {
            QStringList sl = line.split(" ", QString::SkipEmptyParts);
            QString key = sl[1].mid(2);
            QString value = sl[2].mid(2);

//            qDebug() << "KEY : " << key << ", VALUE : " << value;

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

    return lineCount;
}

size_t OSMWorker::filter(const QString &key, const QString &value, const QString& filename, bool bStartsWith)
{
    _resultOutput.clear();

    auto comparerStartsWith = [key, value](const QMap<QString,QString>::const_iterator& it)
    {
        return it.key() == key && it.value().startsWith(value, Qt::CaseInsensitive);
    };

    auto comparerEquals = [key, value](const QMap<QString,QString>::const_iterator& it)
    {
        return it.key() == key && it.value() == value;
    };

    std::function<bool(const QMap<QString,QString>::const_iterator&)> comparerFunction;

    if( bStartsWith )
        comparerFunction = comparerStartsWith;
    else
        comparerFunction = comparerEquals;

    for( const auto& wayPoint : _allWayPoints)
    {
        bool bFound(false);

        for(auto it = wayPoint.keyValues.begin(); !bFound && it != wayPoint.keyValues.end(); ++it)
            bFound = comparerFunction(it);

        if( !bFound )
            continue;

        std::unique_ptr<WAYPOINT> wp( new WAYPOINT);

        auto itName = wayPoint.keyValues.find(QStringLiteral("Name"));

        if( itName != wayPoint.keyValues.end())
        {
            QLatin1String str(itName.value().toLatin1());
            wp->tags.push_back(std::make_pair(str.size(), str));
        }

        int ptsSize = wayPoint.pts.size();
        for(int i =0; i < ptsSize; ++i)
        {
            NODE& node = _allNodes[wayPoint.pts[i]];
            wp->pt.push_back(std::make_pair(node.Lat, node.Lng));

            if( i )
            {
                NODE& prevNode = _allNodes[wayPoint.pts[i-1]];

                GPSLocation from(prevNode.Lat, prevNode. Lng);
                GPSLocation to(node.Lat, node. Lng);

                wp->bearings.push_back(from.bearingTo(to));
                wp->distances.push_back( from.distanceTo(to));
            }
        }

        _resultOutput.push_back(std::move(wp));
    }

    qDebug() << "--------------------------------";
    qDebug() << "Filter Count : " << _resultOutput.size();
    qDebug() << "Key : " << key;
    qDebug() << "Value : " << value;

    if( _resultOutput.size())
    {
        QFile output(filename);
        output.open(QIODevice::WriteOnly);

        QDataStream stream(&output);

        stream << _resultOutput.size();

        for(const auto& item : _resultOutput)
        {
            stream << item->tags.size();

            for( const auto& tag : item->tags)
                stream << tag.first << tag.second.data();

            stream << item->pt.size();

            for( const auto& pt : item->pt)
                stream << pt.first << pt.second;

            for( const auto& pt : item->distances)
                stream << pt;

            for( const auto& pt : item->bearings)
                stream << pt;
        }
    }

    return _resultOutput.size();
}

void OSMWorker::testOSMBin(const QString &filename)
{
    _resultOutput.clear();

    QFile input(filename);
    input.open(QIODevice::ReadOnly);

    QDataStream stream(&input);

    size_t count;
    stream >> count;


    for( size_t i = 0; i < count; ++i)
    {
        std::unique_ptr<WAYPOINT> wp(new WAYPOINT);

        size_t tagCount;
        stream >> tagCount;

        wp->tags.resize(tagCount);

        for(int i = 0; i < tagCount; ++i)
        {
            size_t len;
            stream >> len;
            QByteArray buffer(len, Qt::Uninitialized);
            stream >> buffer;
            wp->tags[i].first = len;
            wp->tags[i].second = QLatin1String(buffer);
        }

        size_t ptsCount;
        stream >> ptsCount;

        wp->pt.resize(ptsCount);
        if( ptsCount > 1)
        {
            wp->bearings.resize(ptsCount-1);
            wp->distances.resize(ptsCount-1);
        }

        for( int i = 0; i < ptsCount; ++i)
            stream >> wp->pt[i].first >> wp->pt[i].second;

        for( int i = 0; i < ptsCount-1; ++i)
            stream >> wp->distances[i];

        for( int i = 0; i < ptsCount-1; ++i)
            stream >> wp->bearings[i];

        _resultOutput.push_back(std::move(wp));
    }

}
