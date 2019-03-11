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

const QString appID = "6fb298fd";
const QString key = "b9434ccf3448ff8def9d55707ed9c406";

TFLRouteWorker::TFLRouteWorker(QObject *parent) : QObject(parent)
{
    _networkManager = new QNetworkAccessManager(this);

    QDir dir(QDir::current());

    dir.mkdir("routes");
    dir.mkdir("stoppoints");

    connect( _networkManager, &QNetworkAccessManager::finished, this, [this](QNetworkReply* reply)
    {
        reply->deleteLater();

        if( reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Internet failed : " << reply->errorString();
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

void TFLRouteWorker::downloadAllRoutesList(bool bInbound)
{
    _bInbound = bInbound;
    beginWork();
}

void TFLRouteWorker::downloadAllStopPoints()
{
    _bDownloadRoutes = false;
    beginWork();
}

void TFLRouteWorker::beginWork()
{
    QUrl url(rootURL);
    QNetworkRequest request(url);
    _networkManager->get(request);
}

void TFLRouteWorker::storeAllRouteIDsInList(const QByteArray &json)
{
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonArray arr = document.array();

    _allRoutesList.clear();

    for( QJsonValue value : arr)
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

    QString lineId = rootObj["lineId"].toString();

    QFile file(QString("routes/%2_%1.txt").arg(lineId).arg(_bInbound?"i":"o"));
    if( !file.open(QIODevice::WriteOnly))
    {
        emit progressSoFar("Failed : " + lineId);
        return;
    }

    QString msg = QString( "Saved: %1 -- Routes Left : %2").arg(file.fileName()).arg(_allRoutesList.size());
    emit progressSoFar(msg);

    QTextStream textStream(&file);
    textStream << json;
    file.close();
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

    QString lineId = _allRoutesList.front();
    _allRoutesList.pop_front();

    QUrl url((QString("https://api.tfl.gov.uk/Line/%1/Route/sequence/%2").arg(lineId).arg(_bInbound?"Inbound":"Outbound")));
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

    for(QJsonValue value : lines)
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

    QFile file(QString("stoppoints/%1.txt").arg(_currentLineId));
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

    QUrl url((QString("https://api.tfl.gov.uk/Line/%1/stoppoints").arg(_currentLineId)));
    url.setQuery(query);
    QNetworkRequest request(url);
    _networkManager->get(request);
}
