#include "TFLRouteWorker.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QDirIterator>

const QString appID = "6fb298fd";
const QString key = "b9434ccf3448ff8def9d55707ed9c406";

TFLRouteWorker::TFLRouteWorker(QObject *parent) : QObject(parent)
{
    _networkManager = new QNetworkAccessManager(this);

    connect( _networkManager, &QNetworkAccessManager::finished, this, [this](QNetworkReply* reply)
    {
        reply->deleteLater();

        if( reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Internet failed : " << reply->errorString();

            if( _bDownloadRoutes )
                downloadNextLine();
            else
                downloadNextStops();

            return;
        }

        QByteArray data = reply->readAll();

        if( reply->request().url().toString() == rootURL)
        {
            storeAllRouteIDsInList(data);

            if( _bDownloadRoutes )
                downloadNextLine();
            else
                downloadNextStops();
        }
        else
        {
            if( _bDownloadRoutes )
            {
                processRoute(data);
                downloadNextLine();
            }
            else
            {
                processStops(data);
                downloadNextStops();
            }
        }
    });
}

void TFLRouteWorker::downloadAllRoutesList(bool bInbound, bool busOnly)
{
    _bInbound = bInbound;
    _bBusOnly = busOnly;
    beginWork();
}

void TFLRouteWorker::downloadAllStopPoints()
{
    _bDownloadRoutes = false;
    beginWork();
}


void TFLRouteWorker::buildAllStopPointsFromRoute(const QString& line, const QByteArray &json, std::map<QString,std::unique_ptr<StopPoint>>& stops)
{
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonObject rootObj = document.object();

    QJsonArray inBranchArray = rootObj["stopPointSequences"].toArray();

    for(const QJsonValue &value : std::as_const(inBranchArray))
    {
        QJsonObject obj;

        QJsonArray inStopPointArray = value["stopPoint"].toArray();
        QJsonArray outStopPointArray;
        for(QJsonValue value : inStopPointArray)
        {
            QString id = value["id"].toString();

            StopPoint* stopPoint = nullptr;
            auto it = stops.find(id);

            if( it == stops.end())
                stopPoint = (stops[id] = std::unique_ptr<StopPoint>(new StopPoint)).get();
            else
                stopPoint = it->second.get();

            stopPoint->id = value["id"].toString();
            stopPoint->stationId = value["stationId"].toString();

            stopPoint->icsId = value["icsId"].toString();
            stopPoint->stopLetter = value["stopLetter"].toString();

            stopPoint->name = value["name"].toString();
            stopPoint->lat = value["lat"].toDouble();
            stopPoint->lon = value["lon"].toDouble();

            stopPoint->lines.push_back(line);
        }
    }
}

void TFLRouteWorker::buildAllStopPointsFromRoutes()
{
    std::map<QString,std::unique_ptr<StopPoint>> allStops;

    QStringList folderList;
    folderList << "inbound" << "outbound";

    for( const QString &folder : std::as_const(folderList))
    {
        QDirIterator dir("data/Routes/" + folder + "/", QDir::Files);
        qDebug() << dir.path();

        while( dir.hasNext())
        {
            QString filename = dir.next();

            QFile file(filename);
            file.open(QIODevice::ReadOnly);

            QByteArray json = file.readAll();

            buildAllStopPointsFromRoute(dir.fileInfo().baseName(), json, allStops);
        }
    }

    qDebug() << "Stops scanned : " << allStops.size();
}

void TFLRouteWorker::mkDirs()
{
    QDir dir(QDir::current());

    dir.mkdir("Routes");
    dir.mkdir("Routes/inbound");
    dir.mkdir("Routes/outbound");
    dir.mkdir("StopPoints");
}

void TFLRouteWorker::beginWork()
{
    mkDirs();
    QUrl url(rootURL);
    QNetworkRequest request(url);
    _networkManager->get(request);
}

void TFLRouteWorker::storeAllRouteIDsInList(const QByteArray &json)
{
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonArray arr = document.array();

    _allRoutesList.clear();

    for( const QJsonValue &value : std::as_const(arr))
    {
        QJsonObject obj = value.toObject();

        QString lineId = obj["id"].toString();
        QString modeName = obj["modeName"].toString();

        _allRoutesList << lineId;
    }
}

void TFLRouteWorker::processRoute(const QByteArray &json)
{
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonObject rootObj = document.object();

    QJsonDocument finalDocument;

    QJsonObject topObject;
    topObject["lineId"] = rootObj["lineId"];
    topObject["lineName"] = rootObj["lineName"];

    QString mode = rootObj["mode"].toString();

    if( _bBusOnly && mode != "bus")
        return;

    topObject["mode"] = mode;

    if( mode != "bus")
        topObject["orderedLineRoutes"] = rootObj["orderedLineRoutes"];

    topObject["direction"] = rootObj["direction"];

    QJsonArray inBranchArray = rootObj["stopPointSequences"].toArray();

    QJsonArray outBranchArray;

    for(const QJsonValue &value : std::as_const(inBranchArray))
    {
        QJsonObject obj;

        QJsonArray inStopPointArray = value["stopPoint"].toArray();
        QJsonArray outStopPointArray;
        for(const QJsonValue &value : std::as_const(inStopPointArray))
        {
            QJsonObject obj;

            obj["parentId"] = value["parentId"];
            obj["stationId"] = value["stationId"];
            obj["icsId"] = value["icsId"];
            obj["topMostParentId"] = value["topMostParentId"];
            obj["stopLetter"] = value["stopLetter"];
            obj["towards"] = value["towards"];
            obj["zone"] = value["zone"];

            obj["id"] = value["id"];
            obj["name"] = value["name"];
            obj["lat"] = value["lat"];
            obj["lon"] = value["lon"];
            outStopPointArray.append(obj);
        }

        obj["stopPoint"] = outStopPointArray;

        outBranchArray.append(obj);
    }

    topObject["stopPointSequences"] = outBranchArray;

    finalDocument.setObject(topObject);

    QDir d = QDir::current();
    d.cd("Routes");
    d.cd(_bInbound?"inbound":"outbound");
    d.mkdir(mode);
    d.cd(mode);

    QFile file(QString("%2/%1.txt").arg(_currentLineId).arg(d.canonicalPath()));
    if( !file.open(QIODevice::WriteOnly))
    {
        emit progressSoFar("Failed : " + _currentLineId);
    }
    else
    {
        QString msg = QString( "Saved: %1 -- Routes Left : %2").arg(file.fileName()).arg(_allRoutesList.size());
        emit progressSoFar(msg);

        QTextStream textStream(&file);
        textStream << finalDocument.toJson(QJsonDocument::Compact);
        file.close();
    }

    d.cdUp();
    d.cdUp();
    d.cdUp();
}

void TFLRouteWorker::downloadNextLine()
{
    if( _allRoutesList.isEmpty())
    {
        emit progressSoFar("ALL ROUTES COMPLETE");
        emit finished();
        return;
    }

    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    _currentLineId = _allRoutesList.front();
    _allRoutesList.pop_front();

    QUrl url((routeSequence.arg(_currentLineId).arg(_bInbound?"Inbound":"Outbound")));
    url.setQuery(query);
    QNetworkRequest request(url);
    _networkManager->get(request);
}

void TFLRouteWorker::processStops(const QByteArray &json)
{
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonObject rootObj = document.object();

    QJsonArray lines = document.array();

    QJsonDocument finalDocument;

    QJsonArray a;

    for(const QJsonValue &value : std::as_const(lines))
    {
        QJsonObject obj;

        obj["naptanId"] = value["naptanId"];
        obj["indicator"] = value["indicator"];
        obj["stopLetter"] = value["stopLetter"];
        obj["icsCode"] = value["icsCode"];
        obj["stationNaptan"] = value["stationNaptan"];
        obj["id"] = value["id"];
        obj["commonName"] = value["commonName"];
        obj["lat"] = value["lat"];
        obj["lon"] = value["lon"];

        a.append(obj);
    }

    finalDocument.setArray(a);

    QFile file(QString("StopPoints/%1.txt").arg(_currentLineId));
    if( !file.open(QIODevice::WriteOnly))
    {
        emit progressSoFar("Failed : " + _currentLineId);
        return;
    }

    QString msg = QString( "Saved: %1 -- Routes Left : %2").arg(file.fileName()).arg(_allRoutesList.size());
    emit progressSoFar(msg);

    QTextStream textStream(&file);
    textStream << finalDocument.toJson(QJsonDocument::Compact);
    file.close();
}

void TFLRouteWorker::downloadNextStops()
{
    if( _allRoutesList.isEmpty())
    {
        emit progressSoFar("ALL STOPS COMPLETE");
        emit finished();
        return;
    }

    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    _currentLineId = _allRoutesList.front();
    _allRoutesList.pop_front();

    QUrl url((lineStopPoints.arg(_currentLineId)));
    url.setQuery(query);
    QNetworkRequest request(url);
    _networkManager->get(request);
}

