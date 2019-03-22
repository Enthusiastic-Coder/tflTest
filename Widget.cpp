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
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include <QVector>


const QString appID = "6fb298fd";
const QString key = "b9434ccf3448ff8def9d55707ed9c406";


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    _manager = new QNetworkAccessManager(this);


    connect( _tflWorker, &TFLRouteWorker::finished, [this]
    {
        ui->pushButtonTFLDownload->setEnabled(true);
        ui->pushButtonStopPoint->setEnabled(true);
    });

    connect( _tflWorker, &TFLRouteWorker::progressSoFar, ui->labelTFLProgress, &QLabel::setText);

//    QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
//    sslConfiguration.setProtocol(QSsl::TlsV1_2);

    connect(ui->pushButtonGETStopPoint, &QPushButton::clicked, [this]
    {
        ui->textBrowser_2->clear();
        ui->labelStopPoint->clear();

        QUrlQuery query;
        query.addQueryItem("app_id", appID);
        query.addQueryItem("app_key", key);

        QString urlText = lineStopPointsURL.arg(ui->comboBoxLines->currentText());

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

            parseStopPoints(doc);
            updateTextBrowserWithStations(ui->textBrowser_2);
            reply->deleteLater();
        });
    });

    connect( ui->pushButtonGET, &QPushButton::clicked, [this]
    {
        ui->textBrowser->clear();

        QUrlQuery query;
        query.addQueryItem("app_id", appID);
        query.addQueryItem("app_key", key);

        QString urlText = lineArrivalsURL.arg(ui->comboBoxLines->currentText());

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

    connect( ui->pushButtonGET_Vehicle, &QPushButton::clicked, [this]
    {
        ui->textBrowser_Vehicle->clear();

        QUrlQuery query;
        query.addQueryItem("app_id", appID);
        query.addQueryItem("app_key", key);

        QString urlText = vehicleArrivalsURL.arg(ui->lineEdit_VehicleID->text());

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

            parseLineArrival(doc, false);
            updateTextBrowserWithArrivals(ui->textBrowser_Vehicle);
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


    connect( ui->pushButtonTFLDownload, &QPushButton::clicked, this, &Widget::startTFLRoutesDownload);

    connect( ui->pushButtonStopPoint, &QPushButton::clicked, this, &Widget::startTFLStopPointDownload);

#ifdef Q_OS_ANDROID
    ui->pushButtonFILE->hide();
#endif

    connect(ui->pushButtonFileSelect, &QPushButton::clicked, [this]{
        QFileDialog *fd = new QFileDialog(this, "OSM file", "/Project/todo/TFL/Tools/data/OSM", "*.osm");
        connect( fd, &QFileDialog::fileSelected, ui->lineEditOSMInputPath, &QLineEdit::setText);
        fd->exec();
        fd->deleteLater();
    });

    connect(ui->pushButtonOSMprocess, &QPushButton::clicked, [this]
    {
        if( ui->lineEditOSMInputPath->text().isEmpty())
        {
            QMessageBox::warning(this, "No File Specified", "Make sure to select an OSM file first", QMessageBox::Close);
            return;
        }

        qlonglong lineCount = _osmWorker->process(ui->lineEditOSMInputPath->text());

        ui->labelOSMProcessLineCount->setText(QString::number(lineCount) + " lines processed.");
    });

    connect(ui->pushButtonOSMfilter, &QPushButton::clicked, [this]
    {
        if( ui->lineEditOSMKey->currentText().isEmpty() || ui->lineEditOSMValue->currentText().isEmpty() || ui->lineEditOSMOutfilename->text().isEmpty())
        {
            QMessageBox::warning(this, "Key/Value blank", "Make sure to set Key/Value", QMessageBox::Close);
            return;
        }

        QFileInfo fi(ui->lineEditOSMInputPath->text());

        QString fullPath = ui->lineEditOSMOutfilename->text();
        fullPath.append("/");
        fullPath.append(fi.baseName());
        fullPath.append(QString("_%1_%2.bin").arg(ui->lineEditOSMKey->currentText()).arg(ui->lineEditOSMValue->currentText()));

        ui->labelOSMFilenameResult->setText( fullPath);
        size_t filterCount = _osmWorker->filter(ui->lineEditOSMKey->currentText(),
                                                    ui->lineEditOSMValue->currentText(),
                                                fullPath,
                                                ui->checkBoxOSMValueStartsWith->isChecked(),
                                                ui->checkBoxOSMFilterOn->isChecked()
                                                );

        ui->labelOSMProcessFilterCount->setText(QString::number(filterCount) + " objects processed.");
    });

    connect( ui->pushButtonExploreToOSMPath, &QPushButton::clicked, [this]
    {
        if( ui->labelOSMFilenameResult->text().isEmpty())
            return;

        QFileInfo fi(ui->labelOSMFilenameResult->text());
        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.path()));
    });

    connect(ui->pushButtonExploreToOSMBIN, &QPushButton::clicked, [this]
    {
        _osmWorker->testOSMBin(ui->labelOSMFilenameResult->text());
    });

    connect(ui->pushButtonAllStopPointsFromRoutes, &QPushButton::clicked, [this]
    {
        _tflWorker->buildAllStopPointsFromRoutes();
    });

    QSettings s;

    ui->lineEditOSMInputPath->setText(s.value("OSMInputPath").toString());
    ui->lineEditOSMKey->setCurrentText(s.value("OSMKey").toString());
    ui->lineEditOSMValue->setCurrentText(s.value("OSMValue").toString());
    ui->comboBoxLines->setCurrentText(s.value("CurrentLine").toString());
    ui->tabWidget->setCurrentIndex(s.value("MainTabIndex").toInt());
    ui->lineEdit_VehicleID->setText(s.value("VehicleID").toString());
    ui->lineEditOSMOutfilename->setText(s.value("OSMOutfilename").toString());
    ui->checkBoxOSMValueStartsWith->setChecked(s.value("OSMValueStarsWith").toBool());
    ui->checkBoxOSMFilterOn->setChecked(s.value("OSMFilterOn").toBool());
}

Widget::~Widget()
{
    QSettings s;

    s.setValue("OSMInputPath", ui->lineEditOSMInputPath->text());
    s.setValue("OSMKey", ui->lineEditOSMKey->currentText());
    s.setValue("OSMValue", ui->lineEditOSMValue->currentText());
    s.setValue("CurrentLine", ui->comboBoxLines->currentText());
    s.setValue("MainTabIndex", ui->tabWidget->currentIndex());
    s.setValue("VehicleID", ui->lineEdit_VehicleID->text());
    s.setValue("OSMOutfilename", ui->lineEditOSMOutfilename->text());
    s.setValue("OSMValueStarsWith", ui->checkBoxOSMValueStartsWith->isChecked());
    s.setValue("OSMFilterOn", ui->checkBoxOSMFilterOn->isChecked());

    delete ui;
}

void Widget::parseLineArrival(const QJsonDocument &doc, bool bMinimize)
{
    QJsonArray items = doc.array();

    _trains.clear();

    for(const auto i : items)
    {
        QJsonObject obj = i.toObject();

        QString id = obj["id"].toString();
        QString vehicleId = obj["vehicleId"].toString();
        QString currentLocation = obj["currentLocation"].toString();
        QString stationName = obj["stationName"].toString();
        int timeToStation = obj["timeToStation"].toInt();
        QString dest = obj["destinationName"].toString();
        QString platform = obj["platformName"].toString();
        QString direction = obj["direction"].toString();
        QString towards = obj["towards"].toString();
        QString naptanId = obj["naptanId"].toString();
        QString lineId = obj["lineId"].toString();

        QString compoundId = dest + id;

        QString finalId = vehicleId.isEmpty()?compoundId:vehicleId;

        Vehicle& v = _trains[!bMinimize?stationName:"" + finalId+lineId];
        if( v.timeToStation == 0 || timeToStation < v.timeToStation || !bMinimize)
        {
            v.vehicleId = vehicleId;
            v.currentLocation = currentLocation;
            v.towards = towards;
            v.timeToStation = timeToStation;
            v.stationName = stationName;
            v.destinationName = dest;
            v.platformName = platform;
            v.direction = direction;
            v.naptanId = naptanId;
            v.lineId = lineId;
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
        OldStopPoint& sp = _stations[stopName];

        sp.name = stopName;
        sp.naptanId = obj["naptanId"].toString();
        sp.location.fLat = obj["lat"].toDouble();
        sp.location.fLng = obj["lon"].toDouble();
    }
}

void Widget::updateTextBrowserWithArrivals(QTextBrowser* textBrowser)
{
    textBrowser->clear();

    QVector<const Vehicle*> trains;

    for(const auto& item : _trains)
        trains << &item;

    std::sort( trains.begin(), trains.end(), [](const Vehicle* l, const Vehicle* r)
    {
        return l->timeToStation < r->timeToStation;
    });

    for(const auto& item : trains)
    {
        textBrowser->append(item->toString());
        textBrowser->append("\n");
    }

    textBrowser->append(QString("COUNT:%1").arg(trains.size()));
}

void Widget::updateTextBrowserWithStations(QTextBrowser *textBrowser)
{
    textBrowser->clear();

    for(const auto& item : _stations)
        textBrowser->append(item.toString());

    textBrowser->append(QString("COUNT:%1").arg(_stations.size()));
}

void Widget::startTFLRoutesDownload()
{
    ui->pushButtonTFLDownload->setEnabled(false);
    ui->pushButtonStopPoint->setEnabled(false);
    _tflWorker->downloadAllRoutesList(ui->checkBoxInBound->isChecked() );
}

void Widget::startTFLStopPointDownload()
{
    ui->pushButtonTFLDownload->setEnabled(false);
    ui->pushButtonStopPoint->setEnabled(false);
    _tflWorker->downloadAllStopPoints();
}
