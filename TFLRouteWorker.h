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

protected:
    void storeAllRouteIDsInList(const QByteArray &json);
    void processRoute(const QByteArray &json);
    void downloadNextLine();

signals:
    void finished();
    void progressSoFar(QString msg);

public slots:

private:
    QNetworkAccessManager* _networkManager = nullptr;
    const QString rootURL = "https://api.tfl.gov.uk/Line/Route/";
    QStringList _allRoutesList;
    bool _bInbound = false;
};

#endif // TFLROUTEWORKER_H
