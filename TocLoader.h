#ifndef TOCLOADER_H
#define TOCLOADER_H

#include <QString>
#include <QList>
#include <QHash>

// Structure to store Tiploc information (from TiplocV1).
struct TiplocInfo
{
    QString transactionType;    // "Create", "Update", or "Delete"
    QString tiplocCode;         // TIPLOC of the location
    QString nalco;              // National Location Code
    QString stanox;             // Station Number
    QString crsCode;            // Computer Reservation System (3-Alpha Code)
    QString description;        // Short name of the location
    QString tpsDescription;     // Full description of the location
};

// Location data for each stop or intermediate point
struct ScheduleLocation {
    QString locationType;  // LO, LI, LT
    QString tiplocCode;    // TIPLOC code
    QString platform;      // Platform at the location
    QString line;          // Line code
    QString path;          // Path code
    QString arrival;       // Arrival time
    QString publicArrival; // Public arrival time
    QString departure;     // Departure time
    QString publicDeparture; // Public departure time
    QString pass;          // Pass time (if applicable)
};

// Main Schedule structure
struct ScheduleInfo {
    QString serviceCode;
    QString stpIndicator;
    QString transactionType; // Create, Update, or Delete
    QString trainUid;        // Unique identifier for the train
    QString atocCode;        // ATOC Code
    QString applicableTimetable; // Whether the timetable is applicable (Y/N)
    QString trainCategory;   // Train category (e.g., Express Passenger)
    QString powerType;       // Power type (e.g., EMU, DMU)
    QString timingLoad;      // Timing load for the train
    QString speed;           // Maximum speed of the train
    QString operatingCharacteristics; // Operating characteristics
    QString trainClass;      // Train class (e.g., 1st/Std)
    QString sleepers;        // Sleeper service information
    QString reservations;    // Reservations information
    QString serviceBranding; // Branding (e.g., CrossCountry, LNER)
    QString startDate;       // Schedule start date
    QString endDate;         // Schedule end date
    QString trainStatus;
    QString daysRun;         // Days of the week the train runs (1111100 format)

    QList<ScheduleLocation> locations; // List of schedule locations
};

class TocLoader
{
public:
    TocLoader();

    void jsonSplitFullToc(const QString& filePath);
    void generateLocationToc(const QString &filePath);
    void generateScheduleToc(const QString &filePath);

private:
    void clear();
    void loadTocData(const QString &filePath);

    QList<TiplocInfo> tiplocList;
    QList<ScheduleInfo> trainScheduleList;
    QHash<QString,QString> tiplocCodeToStanox;

};

#endif // TOCLOADER_H
