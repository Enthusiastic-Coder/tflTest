#ifndef OSMWORKER_H
#define OSMWORKER_H

#include <QObject>
#include <QMap>
#include <QVector>

#include <GPSTileContainer.h>


struct NODE {
    double Lat;
    double Lng;
    QMap<QString,QString> keyValues;
};

struct WAY {
    QVector<qlonglong> pts;
    QMap<QString,QString> keyValues;
};

struct WAYPOINT
{
    std::vector<std::pair<int,QString>> tags;
    std::vector<std::pair<double,double>> pt;
    std::vector<float> distances;
    std::vector<int> bearings;
};

class OSMWorker : public QObject
{
    Q_OBJECT
public:
    explicit OSMWorker(QObject *parent = nullptr);

    qlonglong process(const QString& filename);
    quint64 filter(const QString& key, const QString& value, const QString &filename, bool bStartsWith, bool bFilterOn);
    void testOSMBin( const QString& filename);
signals:

public slots:

private:
    QMap<qlonglong,NODE> _allNodes;
    QMap<qlonglong,WAY> _allWayPoints;
    std::vector<std::unique_ptr<WAYPOINT>> _resultOutput;

};

#endif // OSMWORKER_H
