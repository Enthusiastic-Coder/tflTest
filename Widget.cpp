#include "Widget.h"
#include "ui_Widget.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QFile>
#include <QFileDialog>

const QString appID = "6fb298fd";
const QString key = "b9434ccf3448ff8def9d55707ed9c406";


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    _manager = new QNetworkAccessManager(this);

//    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
//    sslConfiguration.setProtocol(QSsl::TlsV1_2);

    connect( ui->pushButtonGET, &QPushButton::clicked, [this]
    {
        ui->textBrowser->clear();

        QUrlQuery query;
        query.addQueryItem("app_id", appID);
        query.addQueryItem("app_key", key);

        QString urlText = QString("https://api.tfl.gov.uk/line/%1/arrivals").arg(ui->comboBoxLines->currentText());

        QUrl url(urlText);
        url.setQuery(query);

        QNetworkRequest req(url);
        req.setRawHeader("User-Agent" , "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.60 Safari/537.17");

        QNetworkReply* reply = _manager->get(req);

        connect(reply, &QNetworkReply::finished, this, [reply,this]
        {
            QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );

            QByteArray str = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(str);

            parseLineArrival(doc);
            updateTextBrowserWithArrivals(ui->textBrowser);
            reply->deleteLater();
        });

    });

    connect( ui->pushButtonGET_2, &QPushButton::clicked, [this]
    {
        ui->textBrowser_2->clear();
        ui->labelStopPoint->clear();

        QString fileName = QFileDialog::getOpenFileName(this, "Pick file", "/Project/GIT/TrainTracker/data/StopPoints", "*.*");
        QFile f(fileName);

        f.open(QIODevice::ReadOnly);

        if( f.isOpen())
        {
            ui->labelStopPoint->setText( f.fileName() );
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

            parseStopPoints(doc);
            updateTextBrowserWithStations(ui->textBrowser_2);
        }
    });

    connect( ui->pushButtonFILE, &QPushButton::clicked, [this]
    {
        ui->textBrowser->clear();
        ui->labelArrivals->clear();

        QString fileName = QFileDialog::getOpenFileName(this, "Pick file", "/Project/GIT/TrainTracker/data/Arrivals", "*.*");
        QFile f(fileName);

        f.open(QIODevice::ReadOnly);

        if( f.isOpen())
        {
            ui->labelArrivals->setText( f.fileName());
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

            parseLineArrival(doc);
            updateTextBrowserWithArrivals(ui->textBrowser);
        }
    });



#ifdef Q_OS_ANDROID
    ui->pushButtonFILE->hide();
#endif
}

Widget::~Widget()
{
    delete ui;
}

void Widget::parseLineArrival(const QJsonDocument &doc)
{
    QJsonArray items = doc.array();

    _trains.clear();

    for(const auto i : items)
    {
        QJsonObject obj = i.toObject();

        QString vehicleId = obj["vehicleId"].toString();
        QString currentLocation = obj["currentLocation"].toString();
        QString stationName = obj["stationName"].toString();
        int timeToStation = obj["timeToStation"].toInt();
        QString dest = obj["destinationName"].toString();
        QString platform = obj["platformName"].toString();
        QString direction = obj["direction"].toString();
        QString towards = obj["towards"].toString();

        Vehicle& v = _trains[vehicleId.isEmpty()?stationName:vehicleId];
        if( v.timeToStation == 0 || timeToStation < v.timeToStation )
        {
            v.vehicleId = vehicleId;
            v.currentLocation = currentLocation;
            v.towards = towards;
            v.timeToStation = timeToStation;
            v.stationName = stationName;
            v.destinationName = dest;
            v.platformName = platform;
            v.direction = direction;
//            v.formatCurrentLocation();
        }
    }
}

void Widget::parseStopPoints(const QJsonDocument &doc)
{
    _stations.clear();

    QJsonArray items = doc.array();

    for(const auto i : items)
    {
        QJsonObject obj = i.toObject();
        QString stopName = obj["commonName"].toString();
        StopPoint& sp = _stations[stopName];

        sp.name = stopName;
        sp.location.fLat = obj["lat"].toDouble();
        sp.location.fLng = obj["lon"].toDouble();
    }
}

void Widget::updateTextBrowserWithArrivals(QTextBrowser* textBrowser)
{
    textBrowser->clear();

    for(const auto& item : _trains)
    {
        textBrowser->append(item.toString());
        textBrowser->append("\n");
    }

    textBrowser->append(QString("COUNT:%1").arg(_trains.size()));
}

void Widget::updateTextBrowserWithStations(QTextBrowser *textBrowser)
{
    textBrowser->clear();

    for(const auto& item : _stations)
        textBrowser->append(item.toString());

    textBrowser->append(QString("COUNT:%1").arg(_stations.size()));
}
