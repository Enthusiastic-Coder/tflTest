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
            downloadNextLine();
        }
        else
        {
            processRoute(data);
            downloadNextLine();
        }
    });
}

void TFLRouteWorker::downloadAllRoutesList()
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

    QFile file(QString("routes/route_%1.txt").arg(lineId));
    if( !file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed : " << lineId;
        return;
    }

    qDebug() << "Saved:  " << file.fileName();

    QTextStream textStream(&file);
    textStream << json;
    file.close();
}

void TFLRouteWorker::downloadNextLine()
{
    if( _allRoutesList.isEmpty())
    {
        qDebug() << "ALL ROUTES COMPLETE";
        return;
    }


    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QString lineId = _allRoutesList.front();
    _allRoutesList.pop_front();

    QUrl url((QString("https://api.tfl.gov.uk/Line/%1/Route/sequence/Outbound").arg(lineId)));
    url.setQuery(query);
    QNetworkRequest request(url);
    _networkManager->get(request);
}
