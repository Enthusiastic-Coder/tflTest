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
    unsigned char totalTagCount = 0;
    std::vector<unsigned char> tagWordsLengths;
    std::vector<QLatin1String> tagWords;
    unsigned short ptsCount = 0;
    std::vector<std::pair<double,double>> pt;
    std::vector<float> distances;
    std::vector<int> bearings;
};

class OSMWorker : public QObject
{
    Q_OBJECT
public:
    explicit OSMWorker(QObject *parent = nullptr);

    void process(const QString& filename);
    void filter(const QString& key, const QString& value, const QString &filename, bool bStartsWith);
signals:

public slots:

private:
    QMap<qlonglong,NODE> _allNodes;
    QMap<qlonglong,WAY> _allWayPoints;
    std::vector<std::unique_ptr<WAYPOINT>> _resultOutput;

};

#endif // OSMWORKER_H
