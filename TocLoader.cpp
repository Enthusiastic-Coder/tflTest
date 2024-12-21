#include "TocLoader.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>

TocLoader::TocLoader() {}

void TocLoader::filterElizabethLineTrains(const QString &filePath)
{
    // Load JSON file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();

        // Parse the JSON document from the line
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());

        if (doc.isObject()) {
            QJsonObject jsonObj = doc.object();

            // Initialize structures for each iteration
            TrainInfo trainInfo;
            TrainSchedule trainSchedule;

            // Parse TiplocV1 (for Station Info)
            if (jsonObj.contains("TiplocV1")) {
                QJsonObject tiploc = jsonObj["TiplocV1"].toObject();
                TiplocInfo tiplocInfo;
                if (tiploc.contains("tiploc_code")) {
                    tiplocInfo.tiplocCode = tiploc["tiploc_code"].toString();
                }
                if (tiploc.contains("departure")) {
                    tiplocInfo.departureTime = tiploc["departure"].toString();
                }
                if (tiploc.contains("arrival")) {
                    tiplocInfo.arrivalTime = tiploc["arrival"].toString();
                }
                if (tiploc.contains("platform")) {
                    tiplocInfo.platform = tiploc["platform"].toString();
                }
                // Add this Tiploc to the TrainInfo list of intermediate stops
                trainInfo.intermediateTiplocs.append(tiplocInfo.tiplocCode);
            }

            // Parse JsonScheduleV1 (for Schedule Info)
            if (jsonObj.contains("JsonScheduleV1")) {
                QJsonObject schedule = jsonObj["JsonScheduleV1"].toObject();
                if (schedule.contains("CIF_train_service_code")) {
                    trainSchedule.trainServiceCode = schedule["CIF_train_service_code"].toString();
                }
                if (schedule.contains("schedule_start_date")) {
                    trainSchedule.startDate = schedule["schedule_start_date"].toString();
                }
                if (schedule.contains("schedule_end_date")) {
                    trainSchedule.endDate = schedule["schedule_end_date"].toString();
                }

                if (schedule.contains("schedule_location")) {
                    QJsonArray locations = schedule["schedule_location"].toArray();
                    for (const QJsonValue& location : locations) {
                        QJsonObject locationObj = location.toObject();
                        TiplocInfo locInfo;
                        if (locationObj.contains("tiploc_code")) {
                            locInfo.tiplocCode = locationObj["tiploc_code"].toString();
                        }
                        if (locationObj.contains("departure")) {
                            locInfo.departureTime = locationObj["departure"].toString();
                        }
                        if (locationObj.contains("arrival")) {
                            locInfo.arrivalTime = locationObj["arrival"].toString();
                        }
                        if (locationObj.contains("platform")) {
                            locInfo.platform = locationObj["platform"].toString();
                        }
                        // Add this location to the schedule list
                        trainSchedule.scheduleLocations.append(locInfo);
                    }
                }
            }

            // Add the filtered data to the corresponding QList containers
            trainInfoList.append(trainInfo);
            trainScheduleList.append(trainSchedule);
        }
    }

    file.close();
}
