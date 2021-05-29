#include "TFLRouteCompression.h"
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

TFLRouteCompression::TFLRouteCompression(QObject *parent)
    : QObject(parent)
{

}

void TFLRouteCompression::produceCompressedOutput()
{
    QString output;

    QStringList folderList;
    folderList << "inbound" << "outbound";

    for( QString folder : folderList)
    {
        QDirIterator dir("data/Routes/" + folder + "/", QDir::Files);
        qDebug() << dir.path();

        while( dir.hasNext())
        {
            QString filename = dir.next();

            QFile file(filename);
            file.open(QIODevice::ReadOnly);

            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            output.append(processLineObject(doc));

        }
    }

    emit finished(output);
}

QString TFLRouteCompression::processLineObject(QJsonDocument document)
{
    QString objStr;

    auto addString = [&objStr](const QString& l) {
        objStr  += l + "\n";
    };

    auto addDouble = [&objStr](double d) {
        objStr  += QString::number(d)+ "\n";
    };

    QJsonObject rootObject = document.object();

    addString(rootObject[QStringLiteral("lineId")].toString());
    addString(rootObject[QStringLiteral("lineName")].toString());
    addString(rootObject[QStringLiteral("mode")].toString());
    addString(rootObject[QStringLiteral("direction")].toString());

    QJsonArray sequencesArray = rootObject[QStringLiteral("stopPointSequences")].toArray();

    addString(QString::number(sequencesArray.count()));

    for(QJsonValue obj: sequencesArray)
    {
        QJsonArray stopPointsArray = obj[QStringLiteral("stopPoint")].toArray();

        addString(QString::number(stopPointsArray.count()));

        for(QJsonValue obj: stopPointsArray)
        {
            addString(obj[QStringLiteral("id")].toString());
            addDouble(obj[QStringLiteral("lat")].toDouble());
            addDouble(obj[QStringLiteral("lon")].toDouble());

            addString(obj[QStringLiteral("name")].toString());

            QJsonValue stopPointValue = obj[QStringLiteral("stopLetter")];

            addString( stopPointValue.isString()? stopPointValue.toString():QStringLiteral(""));
        }
    }

    QJsonArray orderedRoutes = rootObject[QStringLiteral("orderedLineRoutes")].toArray();
    addString(QString::number(orderedRoutes.count()));

    for(QJsonValue obj: orderedRoutes)
    {
        QJsonArray stops = obj[QStringLiteral("naptanIds")].toArray();

        addString(QString::number(stops.count()));
        for(QJsonValue obj: stops)
            addString(obj.toString());
    }

    return objStr;
}

