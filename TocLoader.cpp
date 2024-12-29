#include "TocLoader.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>

TocLoader::TocLoader() {}

void TocLoader::clear()
{
    tiplocList.clear();
    trainScheduleList.clear();
    tiplocCodeToStanox.clear();
}

void TocLoader::jsonSplitFullToc(const QString &filePath)
{
    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QFileInfo fi(filePath);

    const QString rootName = fi.fileName();

    const QStringList linesList = {

        "CC", "HX", "SE", "SW", "GN", "XR"
    };

    const QSet<QString> validLines {linesList.begin(), linesList.end()};

    QHash<QString,QSharedPointer<QFile>> outputFilesMap;

    QTextStream in(&inFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());

        if (!doc.isObject())
            continue;

        QJsonObject jsonObj = doc.object();

        if (jsonObj.contains("JsonScheduleV1"))
        {
            QJsonObject scheduleObj = jsonObj["JsonScheduleV1"].toObject();

            const QString atocCode = scheduleObj["atoc_code"].toString();

            if( validLines.constFind(atocCode) == validLines.constEnd())
            {
                continue;
            }
        }

        int count ={};

        for(const auto& key : jsonObj.keys())
        {

            QString keyFilename = "/Project/GIT/TFLTest/gen/"  + rootName + "-" + key  + ".json";

            QSharedPointer<QFile> outFile;

            auto it = outputFilesMap.find(keyFilename);

            if( it == outputFilesMap.end() )
            {
                outFile.reset(new QFile{keyFilename});
                outFile->open(QIODevice::WriteOnly | QIODevice::Text);
                outputFilesMap[keyFilename] = outFile;
            }
            else
            {
                outFile = it.value();
            }

            QTextStream outStream(outFile.get());
            outStream << line << "\n";

            count++;
            if( count % 2000 == 0)
            {
                for(auto& fileHandle : outputFilesMap)
                {
                    fileHandle->flush();
                }
            }
        }
    }
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
            schedule.stpIndicator = scheduleObj["CIF_stp_indicator"].toString();
            schedule.trainStatus = scheduleObj["train_status"].toString();

            bool doAdd{false};

            if ((schedule.trainStatus == "P" || schedule.trainStatus == "1") &&
                (schedule.stpIndicator == "N" || schedule.stpIndicator == "R" || schedule.stpIndicator == "O"))
            {
                doAdd = true;
            }

            if( !doAdd)
            {
                continue;
            }

            schedule.trainUid = scheduleObj["CIF_train_uid"].toString();
            schedule.atocCode = scheduleObj["atoc_code"].toString();
            schedule.applicableTimetable = scheduleObj["applicable_timetable"].toString();
            schedule.startDate = scheduleObj["schedule_start_date"].toString();
            schedule.endDate = scheduleObj["schedule_end_date"].toString();
            schedule.daysRun = scheduleObj["schedule_days_runs"].toString();

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

        QString departTime, arrivalTime;

        if (!schedule.locations.isEmpty())
        {

            {
                const auto& record = schedule.locations.first();
                firstLocation = record.tiplocCode;
                departTime = record.departure;

                auto itFirst = tiplocCodeToStanox.find(firstLocation);
                if( itFirst != tiplocCodeToStanox.end())
                {
                    firstStanox = itFirst.value();
                }
            }

            {
                const auto& record = schedule.locations.last();
                lastLocation = record.tiplocCode;
                arrivalTime = record.arrival;

                auto itLast = tiplocCodeToStanox.find(lastLocation);
                if( itLast != tiplocCodeToStanox.end())
                {
                    lastStanox = itLast.value();
                }
            }
        }

        out << firstStanox << "," << lastStanox << "," << firstLocation << "," << lastLocation;
        out << "," << departTime << "," << arrivalTime;

        out << "," << schedule.startDate << "," << schedule.endDate;

        out << "," << schedule.daysRun;

        out << "\r\n";
    }
}
