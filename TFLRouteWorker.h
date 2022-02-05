#ifndef TFLROUTEWORKER_H
#define TFLROUTEWORKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <memory>

struct StopPoint
{
    QString icsId;
    QString id;
    float lat;
    float lon;
    QString name;
    QString stationId;
    QString stopLetter;
    QVector<QString> lines;
};

class TFLRouteWorker : public QObject
{
    Q_OBJECT
public:
    explicit TFLRouteWorker(QObject *parent = nullptr);

    void downloadAllRoutesList(bool bInbound=false, bool busOnly=false);
    void downloadAllStopPoints();

    void buildAllStopPointsFromRoutes();

protected:
    void mkDirs();
    void beginWork();
    void storeAllRouteIDsInList(const QByteArray &json);

    void processRoute(const QByteArray &json);
    void downloadNextLine();

    void processStops(const QByteArray &json);
    void downloadNextStops();

    void buildAllStopPointsFromRoute(const QString &line, const QByteArray& json, std::map<QString, std::unique_ptr<StopPoint> > &stops);

signals:
    void finished();
    void progressSoFar(QString msg);

public slots:

private:
    QNetworkAccessManager* _networkManager = nullptr;
    const QString rootURL = "https://api.tfl.gov.uk/Line/Route/";
    const QString routeSequence = "https://api.tfl.gov.uk/Line/%1/Route/sequence/%2";
    const QString lineStopPoints = "https://api.tfl.gov.uk/Line/%1/stoppoints";
    QStringList _allRoutesList;
    bool _bInbound = false;
    bool _bBusOnly = false;
    QString _currentLineId;
    bool _bDownloadRoutes = true;
};

#endif // TFLROUTEWORKER_H
