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
signals:

public slots:
};

#endif // OSMWORKER_H
