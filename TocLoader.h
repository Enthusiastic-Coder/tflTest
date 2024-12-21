#ifndef TOCLOADER_H
#define TOCLOADER_H

#include <QString>
#include <QList>


// Structure to store information about the train.
struct TrainInfo {
    QString trainUid;
    QString serviceCode;
    QString originTiploc;
    QString terminusTiploc;
    QList<QString> intermediateTiplocs;
};

// Structure to store Tiploc information (from TiplocV1).
struct TiplocInfo {
    QString tiplocCode;
    QString departureTime;
    QString arrivalTime;
    QString platform;
    // Other fields from TiplocV1 can be added here if needed
};

// Structure to store Schedule details (from JsonScheduleV1).
struct TrainSchedule {
    QString trainServiceCode;
    QString startDate;
    QString endDate;
    QList<TiplocInfo> scheduleLocations; // List of stops along the schedule
    // Other fields from JsonScheduleV1 can be added here if needed
};


class TocLoader
{
public:
    TocLoader();


    void filterElizabethLineTrains(const QString &filePath);

private:

    QList<TrainInfo> trainInfoList;
    QList<TrainSchedule> trainScheduleList;

};

#endif // TOCLOADER_H
