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

const QString appID = "6fb298fd";
const QString key = "b9434ccf3448ff8def9d55707ed9c406";


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    _manager = new QNetworkAccessManager(this);

    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    sslConfiguration.setProtocol(QSsl::TlsV1_2);

    connect( _manager, &QNetworkAccessManager::finished, this , [this](QNetworkReply* reply)
    {
        QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );


        QByteArray str = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(str);

        parseData(doc);
        reply->deleteLater();
    });

    connect( ui->pushButtonGET, &QPushButton::clicked, [this]
    {
        QUrlQuery query;
        query.addQueryItem("app_id", appID);
        query.addQueryItem("app_key", key);

        QUrl url(QString("https://api.tfl.gov.uk/line/%1/arrivals").arg(ui->comboBoxLines->currentText()));
        url.setQuery(query);

        QNetworkRequest req(url);
        req.setRawHeader("User-Agent" , "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.60 Safari/537.17");

        _manager->get(req);

    });

    connect( ui->pushButtonFILE, &QPushButton::clicked, [this]
    {
        QFile f("/project/todo/tfl/sample-picc.txt");

        f.open(QIODevice::ReadOnly);

        if( f.isOpen())
        {
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

            parseData(doc);
        }
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::parseData(const QJsonDocument &doc)
{
    QJsonArray items = doc.array();

    _trains.clear();

    for(const auto& i : items)
    {
        QJsonObject obj = i.toObject();
        QString id = obj["vehicleId"].toString();
        QString currentLocation = obj["currentLocation"].toString();
        QString stationName = obj["stationName"].toString();
        int timeToStation = obj["timeToStation"].toInt();

        Vehicle& v = _trains[id];
        if( v.timeToStation == 0 || v.timeToStation > timeToStation)
        {
            v.vehicleId = id;
            v.currentLocation = currentLocation;
            v.timeToStation = timeToStation;
            v.stationName = stationName;
        }
    }

    updateTextBrowserWithMap();
}

void Widget::updateTextBrowserWithMap()
{
    ui->textBrowser->clear();

    for(const auto& item : _trains)
    {
        ui->textBrowser->append(item.toString());
        ui->textBrowser->append("\n---------\n");
    }

    ui->textBrowser->append(QString("COUNT:%1").arg(_trains.size()));
}

