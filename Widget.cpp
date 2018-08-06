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

        int ne = reply->error();

        int status = 0;

        if ( statusCode.isValid() )
            status = statusCode.toInt();

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

        QUrl url(QString("https://api.tfl.gov.uk/line/%1/arrivals").arg(ui->lineEdit->text()));
        url.setQuery(query);

        QNetworkRequest req(url);
        req.setRawHeader("User-Agent" , "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.60 Safari/537.17");

        _manager->get(req);

    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::parseData(const QJsonDocument &doc)
{
    QJsonArray items = doc.array();

    for(const auto& i : items)
    {
        QJsonObject obj = i.toObject();
        QString id = obj["vehicleId"].toString();
        QString currentLocation = obj["currentLocation"].toString();

    }
}

