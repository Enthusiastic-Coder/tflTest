#include "OSMWorker.h"

#include <iostream>
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

    QMap<qlonglong,NODE> allNodes;

    QMap<qlonglong,WAY> allWayPoints;

    QTextStream stream(&file);

    int lineCount(0);
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

            allNodes[nodeId] = node;
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

            WAY& way = allWayPoints[currentWayID];
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
                NODE& node = allNodes[currentNodeID];
                node.keyValues[key] = value;
            }
            else
            {
                WAY& way = allWayPoints[currentWayID];
                way.keyValues[key] = value;
            }
        }
        else if( line.startsWith(QLatin1String("<relation")))
            break;

        if( lineCount % 100000 == 0)
            qDebug() << "Line count : " << lineCount;

    }

    std::cout << "Nodes Counted : " << allNodes.size() << "\n";
    std::cout << "Waypoints Counted : " << allWayPoints.size() << "\n";
    std::cout << "Seconds : " << t.elapsed()/1000<< "\n";

}
