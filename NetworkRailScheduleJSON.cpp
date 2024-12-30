#include "NetworkRailScheduleJSON.h"

#include <QFile>
#include <QJsonDocument>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>


void NetworkRailScheduleJSON::load(const QString &filename)
{
    _services.clear();

    QFile file(filename);
    if( !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << Q_FUNC_INFO << "- load **failed**";
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if( doc.isNull())
    {
        qDebug() << Q_FUNC_INFO << "- fromJson **failed**";
        return;
    }

    QDate today = QDate::currentDate();

    QJsonArray services = doc.array();

    for(const auto& service: std::as_const(services))
    {
        QJsonObject serviceObj = service.toObject();

        QDate startDate = QDate::fromString(serviceObj["startDate"].toString(), "yyyy-MM-dd");
        QDate endDate = QDate::fromString(serviceObj["endDate"].toString(), "yyyy-MM-dd");

        if( today < startDate || today > endDate) {
            continue;
        }

        const QString daysRun = serviceObj["daysRun"].toString();

        QBitArray bitArray(7);

        for (int i = 0; i < 7; ++i) {
            bitArray.setBit(i, daysRun.at(i) == '1');
        }

        if( !bitArray.testBit(today.dayOfWeek()-1)) {
            continue;
        }

        QString atoc = serviceObj["atoc"].toString();
        QString serviceCode = serviceObj["serviceCode"].toString();

        QJsonArray stns = serviceObj["stns"].toArray();

        NRScheduleDATA serviceDATA;
        serviceDATA.serviceCode = serviceCode;
        serviceDATA.atocCode = atoc;
        serviceDATA.startDate = startDate;
        serviceDATA.endDate = endDate;
        serviceDATA.destinationStanox = stns.last()["stanox"].toString();

        for(const auto&stn : std::as_const(stns))
        {
            QJsonObject pointObj = stn.toObject();

            serviceDATA.stations[pointObj["stanox"].toString()]
                = QTime::fromString(pointObj["arrivalTime"].toString(), "hhmm");
        }

        _services.insert(atoc + "|" + serviceCode, serviceDATA);
    }
}

QString NetworkRailScheduleJSON::getDestination(const QString &atoccode, const QString &serviceCode, const QString &stanox, const QTime &now) const
{
    const auto& servicesAvailable = _services.values(atoccode +"|"+ serviceCode);

    const NRScheduleDATA* foundService = nullptr;
    int timeDiff = -1;

    for(const auto& service : servicesAvailable)
    {
        QTime arrivalTime = service.stations.value(stanox);

        if( !arrivalTime.isValid())
        {
            continue;
        }

        if( timeDiff < 0)
        {
            foundService = &service;
            timeDiff = qAbs(now.secsTo(arrivalTime));
        }
        else
        {
            if( qAbs(now.secsTo(arrivalTime)) < timeDiff)
            {
                foundService = &service;
            }
        }
    }

    if( foundService )
    {
        return foundService->destinationStanox;
    }

    return {};
}
