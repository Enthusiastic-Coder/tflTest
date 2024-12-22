#include "TocLoader.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>

TocLoader::TocLoader() {}

void TocLoader::clear()
{
    tiplocList.clear();
    trainScheduleList.clear();
    tiplocCodeToStanox.clear();
}

void TocLoader::loadTocData(const QString &filePath)
{
    clear();

    // Load JSON file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        // Parse the JSON document from the line
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());

        if (!doc.isObject())
            continue;

        QJsonObject jsonObj = doc.object();

        // Check if the JSON contains TiplocV1
        if (jsonObj.contains("TiplocV1"))
        {
            QJsonObject tiplocObj = jsonObj["TiplocV1"].toObject();
            TiplocInfo tiploc;

            tiploc.transactionType = tiplocObj["transaction_type"].toString();
            tiploc.tiplocCode = tiplocObj["tiploc_code"].toString();
            tiploc.nalco = tiplocObj["nalco"].toString();
            tiploc.stanox = tiplocObj["stanox"].toString();
            tiploc.crsCode = tiplocObj["crs_code"].toString();
            tiploc.description = tiplocObj["description"].toString();
            tiploc.tpsDescription = tiplocObj["tps_description"].toString();


            tiplocCodeToStanox[tiploc.tiplocCode] = tiploc.stanox;


            tiplocList.append(tiploc);
        }

        if (jsonObj.contains("JsonScheduleV1"))
        {
            QJsonObject scheduleObj = jsonObj["JsonScheduleV1"].toObject();

            ScheduleInfo schedule;

            // Basic schedule details
            schedule.transactionType = scheduleObj["transaction_type"].toString();
            schedule.trainUid = scheduleObj["CIF_train_uid"].toString();
            schedule.atocCode = scheduleObj["atoc_code"].toString();
            schedule.applicableTimetable = scheduleObj["applicable_timetable"].toString();
            schedule.startDate = scheduleObj["schedule_start_date"].toString();
            schedule.endDate = scheduleObj["schedule_end_date"].toString();
            schedule.daysRun = scheduleObj["schedule_days_runs"].toString();
            schedule.trainStatus = scheduleObj["train_status"].toString();

            // Schedule segment details
            QJsonObject segment = scheduleObj["schedule_segment"].toObject();
            schedule.serviceCode = segment["CIF_train_service_code"].toString();
            schedule.trainCategory = segment["CIF_train_category"].toString();
            schedule.powerType = segment["CIF_power_type"].toString();
            schedule.timingLoad = segment["CIF_timing_load"].toString();
            schedule.speed = segment["CIF_speed"].toString();
            schedule.operatingCharacteristics = segment["CIF_operating_characteristics"].toString();
            schedule.trainClass = segment["CIF_train_class"].toString();
            schedule.sleepers = segment["CIF_sleepers"].toString();
            schedule.reservations = segment["CIF_reservations"].toString();
            schedule.serviceBranding = segment["CIF_service_branding"].toString();

            // Parse locations
            if (segment.contains("schedule_location"))
            {
                QJsonArray locationsArray = segment["schedule_location"].toArray();
                for (const QJsonValue &locValue : locationsArray)
                {
                    QJsonObject locObj = locValue.toObject();
                    ScheduleLocation location;

                    location.locationType = locObj["location_type"].toString();
                    location.tiplocCode = locObj["tiploc_code"].toString();
                    location.platform = locObj["platform"].toString();
                    location.line = locObj["line"].toString();
                    location.path = locObj["path"].toString();
                    location.arrival = locObj["arrival"].toString();
                    location.publicArrival = locObj["public_arrival"].toString();
                    location.departure = locObj["departure"].toString();
                    location.publicDeparture = locObj["public_departure"].toString();
                    location.pass = locObj["pass"].toString();

                    schedule.locations.append(location);
                }
            }

            trainScheduleList.push_back(schedule);
        }
    }

    file.close();
}

void TocLoader::generateFilteredFile(const QString &filePath)
{
    // Load JSON file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QFile outFile("data/NetworkRail/filtered_toc.txt");
    if( !outFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QTextStream out(&outFile);

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine();

        // Parse the JSON document from the line
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());

        if (!doc.isObject())
            continue;

        QJsonObject jsonObj = doc.object();

        // Check if the JSON contains TiplocV1
        if (jsonObj.contains("TiplocV1"))
        {
            out << line << "\r\n";
        }

        if (jsonObj.contains("JsonScheduleV1"))
        {
            QJsonObject scheduleObj = jsonObj["JsonScheduleV1"].toObject();
            // Filter for Elizabeth Line (optional, based on ATOC Code or other criteria)

            if (scheduleObj["atoc_code"].toString() == "XR")
            {
                out << line << "\r\n";
            }
        }
    }
}

void TocLoader::generateLocationToc(const QString &filePath)
{
    loadTocData(filePath);

    QFile outFile("data/NetworkRail/location-toc.txt");
    if( !outFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QTextStream out(&outFile);

    for(const auto& location: std::as_const(tiplocList))
    {
        if(location.stanox.isEmpty())
            continue;

        out << location.stanox << ","
            << location.tpsDescription << ","
            << location.crsCode << ","
            << location.description << ","
            << location.tiplocCode << "\r\n";
    }
}

void TocLoader::generateScheduleToc(const QString &filePath)
{
    loadTocData(filePath);

    QFile outFile("data/NetworkRail/schedule-toc.txt");
    if( !outFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QTextStream out(&outFile);

    for(const auto& schedule: std::as_const(trainScheduleList))
    {
        out << schedule.atocCode << ",";
        out << schedule.serviceCode << ",";

        QString firstLocation, firstStanox;
        QString lastLocation, lastStanox;

        if (!schedule.locations.isEmpty())
        {

            {
                firstLocation = schedule.locations.first().tiplocCode;
                auto itFirst = tiplocCodeToStanox.find(firstLocation);
                if( itFirst != tiplocCodeToStanox.end())
                {
                    firstStanox = itFirst.value();
                }
            }

            {
                lastLocation = schedule.locations.last().tiplocCode;

                auto itLast = tiplocCodeToStanox.find(lastLocation);
                if( itLast != tiplocCodeToStanox.end())
                {
                    lastStanox = itLast.value();
                }
            }
        }

        out << firstStanox << ",";
        out << lastStanox << ",";

        out << firstLocation << ",";
        out << lastLocation;

        out << "," << schedule.startDate << "," << schedule.endDate;

        out << "," << schedule.daysRun << "," << schedule.trainStatus;

        out << "\r\n";
    }
}
