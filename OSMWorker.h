#ifndef OSMWORKER_H
#define OSMWORKER_H

#include <QObject>
#include <QMap>
#include <QVector>


struct NODE {
    double Lat;
    double Lng;
    QMap<QString,QString> keyValues;
};

struct WAY {
    QVector<qlonglong> pts;
    QMap<QString,QString> keyValues;
};

class OSMWorker : public QObject
{
    Q_OBJECT
public:
    explicit OSMWorker(QObject *parent = nullptr);

    void process(const QString& filename);
    void filter(const QString& key, const QString& value);
signals:

public slots:

private:
    QMap<qlonglong,NODE> _allNodes;
    QMap<qlonglong,WAY> _allWayPoints;

};

#endif // OSMWORKER_H
