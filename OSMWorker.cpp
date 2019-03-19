#include "OSMWorker.h"

#include <QElapsedTimer>
#include <QFile>
#include <QDataStream>
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
                    itemsObtained++;
                }
                if( item.startsWith(QStringLiteral("lon=")))
                {
                    node.Lng = item.mid(4).toDouble();

                    if( node.Lng == 0)
                    {
                        qDebug() << "HELLO";
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
}

void OSMWorker::filter(const QString &key, const QString &value, const QString& filename, bool bStartsWith)
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
            wp->totalTagCount = 1;
            QLatin1String str(itName.value().toLatin1());
            wp->tagWords.push_back(str);
            wp->tagWordsLengths.push_back(str.size());
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

        wp->ptsCount = wp->pt.size();
        _resultOutput.push_back(std::move(wp));
    }

    qDebug() << "--------------------------------";
    qDebug() << "Filter Count : " << _resultOutput.size();
    qDebug() << "Key : " << key;
    qDebug() << "Value : " << value;

    if( _resultOutput.size() == 0)
        return;

    QFile output(filename);
    output.open(QIODevice::WriteOnly);

    QDataStream stream(&output);

    for(const auto& item : _resultOutput)
    {
        stream << item->totalTagCount;

        for( const auto& tagLength : item->tagWordsLengths)
            stream << tagLength;

        for( const auto& tagWord : item->tagWords)
            stream << tagWord.data();

        stream << item->ptsCount;

        for( const auto& pt : item->pt)
            stream << pt.first << pt.second;

        for( const auto& pt : item->distances)
            stream << pt;

        for( const auto& pt : item->bearings)
            stream << pt;
    }
}
