#ifndef TFLROUTEWORKER_H
#define TFLROUTEWORKER_H

#include <QObject>
#include <QNetworkAccessManager>

class TFLRouteWorker : public QObject
{
    Q_OBJECT
public:
    explicit TFLRouteWorker(QObject *parent = nullptr);

    void downloadAllRoutesList(bool bInbound=false);
    void downloadAllStopPoints();

protected:
    void beginWork();
    void storeAllRouteIDsInList(const QByteArray &json);

    void processRoute(const QByteArray &json);
    void downloadNextLine();

    void processStops(const QByteArray &json);
    void downloadNextStops();

signals:
    void finished();
    void progressSoFar(QString msg);

public slots:

private:
    QNetworkAccessManager* _networkManager = nullptr;
    const QString rootURL = "https://api.tfl.gov.uk/Line/Route/";
    QStringList _allRoutesList;
    bool _bInbound = false;
    QString _currentLineId;
    bool _bDownloadRoutes = true;
};

#endif // TFLROUTEWORKER_H
