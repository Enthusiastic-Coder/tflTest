#include "Widget.h"
#include "TocLoader.h"
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
#include <QString>
#include <set>
#include <QRgb>

const QString appID = "6fb298fd";
const QString key = "b9434ccf3448ff8def9d55707ed9c406";

//const QString ADSBExchange_URL = "https://flighttracker-app.adsbexchange.com/VirtualRadar/AircraftList.json";

const QString ADSBExchange_URL = "adsbexchange.com/VirtualRadar/AircraftList.json";


std::string encryptDecrypt(const std::string& toEncrypt, const std::string& salt)
{
    std::string output = toEncrypt;

    for (int i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ salt[i];

    return output;
}


QString GetRandomString(int randomStringLength)
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
//       int index = qrand() % 128+33;
       int index = std::rand() % possibleCharacters.size();
       QChar nextChar = QChar(possibleCharacters.at(index));
       randomString.append(nextChar);
   }
   return randomString;
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    _osmTileGenerator(new OSMTileGenerator(this))
{
    ui->setupUi(this);
    _osmTileGenerator->setUp(ui);
    _manager = new QNetworkAccessManager(this);

    _networkRailStnCSV.Load( "data/NetworkRail/network_rail_stns.txt", 3);

    _networkRailScheduleJSON.load("data/NetworkRail/schedule-toc.json");

    _NRStatusTimer->start(1000);
    connect( _NRStatusTimer, &QTimer::timeout, [this] {
        ui->labelStatus_NR->setText( _client.isUnconnected()?"Disconnected":"Active");
    });

    connect(ui->pushButton_NetworkRail, &QPushButton::toggled, [this](bool toggled) {

        ui->comboBox_NetworkRail->setDisabled(toggled);
        if( toggled)
        {
            ui->pushButton_NetworkRail->setText("Stop");
            _client.connectToHost("publicdatafeeds.networkrail.co.uk", 61618);
        }
        else
        {
            _client.disconnectFromHost();
            ui->pushButton_NetworkRail->setText("Start");
        }
    });

    QObject::connect(&_client, &QStompClient::socketConnected, [this] {
        qDebug() << "Connected";

        QTimer *heartbeatTimer = new QTimer(this);
        connect(heartbeatTimer, &QTimer::timeout, this, [&]() {
            _client.send(QByteArray(), QString("\n"));
        });
        heartbeatTimer->start(5000);

        _client.login("jebaramo@gmail.com", "6MHAk3Nmy!tL7NQ");

        ui->textBrowser_NetworkRail->append("Connected");

        QString id = QString("/topic/TRAIN_MVT_%1_TOC").arg(ui->comboBox_NetworkRail->currentText());

        // id = "/topic/TD_ALL_SIG_AREA";

        ui->textBrowser_NetworkRail->append("Subscribing to " + id );
        _client.subscribe(id.toLocal8Bit(), true);
    });

    QObject::connect(&_client, &QStompClient::socketDisconnected,  [this] {
        ui->textBrowser_NetworkRail->append("Dis-Connected");
    });

    connect(&_client, &QStompClient::frameReceived, [this] {

        QStompResponseFrame frame = _client.fetchFrame();
        QByteArray str = frame.body().toLocal8Bit();
        QJsonDocument doc = QJsonDocument::fromJson(str);

        if( !doc.isNull() && !ui->checkBox_NetworkRail->isChecked())
            parseNetworkRail(doc);
        else
            ui->textBrowser_NetworkRail->append(doc.toJson(QJsonDocument::Indented));
    });

    connect(ui->pushButton_Clear_NR, &QPushButton::pressed, [this] {
        ui->textBrowser_NetworkRail->clear();
    });


    connect(ui->pushButton_SplitTocFull, &QPushButton::pressed, [this]  {

        TocLoader loader;

        QString fileName = QFileDialog::getOpenFileName(this,
                                                        "Split Toc File",
                                                        "/Project/GIT/TFLTest/gen",
                                                        "*.*");

        if( fileName.length() )
        {
            loader.jsonSplitFullToc(fileName);
        }

        QMessageBox::information(0, "jsonSplitFullToc", "Completed.");

    });

    connect(ui->pushButton_GenLoc, &QPushButton::pressed, [this] {

        TocLoader loader;


        QString fileName = QFileDialog::getOpenFileName(this,
                                                        "Generate Location",
                                                        "/Project/GIT/TFLTest/gen",
                                                        "*.*");

        if( fileName.length() )
        {
            loader.generateLocationToc(fileName);
        }

        QMessageBox::information(0, "generateLocationToc", "Completed.");
    });

    connect(ui->pushButton_GenSchedCSV, &QPushButton::pressed, [this] {

        TocLoader loader;

        QString fileName = QFileDialog::getOpenFileName(this,
                                                        "Generate Schedule CSV",
                                                        "/Project/GIT/TFLTest/gen",
                                                        "*.*");

        if( fileName.length() )
        {
            loader.generateScheduleTocCSV(fileName);
        }


        QMessageBox::information(0, "Generate Schedule CSV", "Completed.");
    });

    connect(ui->pushButton_GenSchedJSON, &QPushButton::pressed, [this] {

        TocLoader loader;

        QString fileName = QFileDialog::getOpenFileName(this,
                                                        "Generate Schedule JSON",
                                                        "/Project/GIT/TFLTest/gen",
                                                        "*.*");

        if( fileName.length() )
        {
            loader.generateScheduleTocJSON(fileName, ui->lineEditScheduleJSONFilter->text());
        }


        QMessageBox::information(0, "Generate Schedule JSON", "Completed.");
    });


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

            if( reply->error() == QNetworkReply::NoError)
            {
                QByteArray str = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(str);

                parseStopPoints(doc);
                updateTextBrowserWithStations(ui->textBrowser_2);
            }
            else
            {
                ui->textBrowser_2->setText(reply->errorString());
            }

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

            if( reply->error() == QNetworkReply::NoError)
            {
                QByteArray str = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(str);

                parseLineArrival(doc);
                updateTextBrowserWithArrivals(ui->textBrowser);
            }
            else
            {
                ui->textBrowser->setText(reply->errorString());
            }

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

            if( reply->error() == QNetworkReply::NoError)
            {
                QByteArray str = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(str);

                parseLineArrival(doc, false);
                updateTextBrowserWithArrivals(ui->textBrowser_Vehicle);
            }
            else
            {
                ui->textBrowser_Vehicle->setText(reply->errorString());
            }

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
        QMessageBox::information(this, "OSM Process", "Complete");
    });

    connect(ui->pushButtonOSMfilter, &QPushButton::clicked, [this]
    {
        if( ui->lineEditOSMOutfilename->text().isEmpty())
        {
            QMessageBox::warning(this, "Output folder", "Make sure to set destination folder.", QMessageBox::Close);
            return;
        }

        QFileInfo fi(ui->lineEditOSMInputPath->text());

        QString osmKey = ui->lineEditOSMKey->currentText();
        QString osmValue = ui->lineEditOSMValue->currentText();

        QString fullPath = ui->lineEditOSMOutfilename->text();
        fullPath.append("/");
        fullPath.append(fi.baseName());

        if( !osmKey.isEmpty())
            fullPath.append(QString("_%1").arg(osmKey));

        if( !osmValue.isEmpty())
            fullPath.append(QString("_%1").arg(osmValue));

        fullPath.append(".bin");

        ui->labelOSMFilenameResult->setText( fullPath);
        quint64 filterCount = _osmWorker->filter(ui->lineEditOSMKey->currentText(),
                                                    ui->lineEditOSMValue->currentText(),
                                                fullPath,
                                                ui->checkBoxOSMValueStartsWith->isChecked(),
                                                ui->checkBoxOSMFilterOn->isChecked()
                                                );

        ui->labelOSMProcessFilterCount->setText(QString::number(filterCount) + " objects processed.");

        QMessageBox::information(this, "OSM Filter", "Complete");
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

    connect(ui->pushButtonUSStates, &QPushButton::clicked, this, &Widget::processUSStates);

    connect(ui->pushButtonXOR, &QPushButton::clicked, [this]
    {
        std::string encryString = encryptDecrypt(ui->lineEditXORText->text().toStdString(),
                                                 ui->lineEditXORSalt->text().toStdString());

        ui->lineEditXORResult->setText(QString::fromStdString(encryString));
    });

    ui->lineEditXORText->setText(ADSBExchange_URL);
    connect(ui->pushButtonXORRandomString, &QPushButton::clicked, [this] {
        ui->lineEditXORRandomString->setText(GetRandomString(ADSBExchange_URL.length()));
    });

    connect(ui->pushButtonXORSave, &QPushButton::clicked, [this] {
        QFile f;
        f.setFileName("old.bin");
        f.open(QIODevice::WriteOnly);
        QByteArray array = ui->lineEditXORResult->text().toLatin1();
        f.write(array);

    });

    connect(ui->pushButtonXORLoad, &QPushButton::clicked, [this] {
        QFile f;
        f.setFileName("old.bin");
        f.open(QIODevice::ReadOnly);
        QByteArray array = f.readAll();
        ui->lineEditXORText->setText(array);
    });

    connect(ui->pushButtonTflRouteGen, &QPushButton::clicked, [this] {

        ui->pushButtonTflRouteGen->setEnabled(false);
        _tflRouteCompress->produceCompressedOutput();
    });

    connect(ui->aircraftjson_pushButton, &QPushButton::clicked, this, &Widget::processAircraftJson);
    connect(ui->operator_json_pushButton, &QPushButton::clicked, this, &Widget::processOperatorJson);

    connect(_tflRouteCompress, &TFLRouteCompression::finished, this, [this](QString output) {
        ui->pushButtonTflRouteGen->setEnabled(true);

        QFile f("TFLCompressed.txt");
        if( f.open(QIODevice::WriteOnly))
        {
            f.write(output.toLatin1());
            QMessageBox::information(this, "TFL Compress done", "TFL compress");
        }
    });

    connect( ui->pushButtonRunwayBlend, &QPushButton::clicked, this, &Widget::runwayBlend );

    double lat =51.492;
    double lng = -0.306202;
    int zoom = 16;

    const TileCoordinates coords = mapGPSToTile(lat, lng, zoom);

    ui->label_osm_txt_generation->setText(QString("Tile: %1/%2").arg(coords.x).arg(coords.y));

    connect( ui->pushButton_OSM_Texture_Generate, &QPushButton::clicked, this, [this] {

        QStringList zoomLevels = ui->plainTextEdit_ZoomLevels->toPlainText().split(",");

        TileCorners corners;

        corners.topLeft.latitude = 51.6849;
        corners.topLeft.longitude = -0.525459;

        corners.bottomRight.latitude = 51.2772;
        corners.bottomRight.longitude = 0.362426;

        for(const QString& zoom: zoomLevels)
        {
            _osmTileDownloader->generate(corners, zoom.toInt(), [this](QString msg) {
                ui->label_osm_txt_generation->setText(msg);
                QCoreApplication::processEvents();
            });
        }

        ui->label_osm_txt_generation->setText("Finished");
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

    s.beginGroup( "mainwindow" );

    restoreGeometry(s.value( "geometry", saveGeometry() ).toByteArray());
    move(s.value( "pos", pos() ).toPoint());
    resize(s.value( "size", size() ).toSize());
    if ( s.value( "maximized", isMaximized() ).toBool() )
        showMaximized();

    s.endGroup();
}

Widget::~Widget()
{
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
        QDateTime expectedArrival = QDateTime::fromString(obj["expectedArrival"].toString(), Qt::ISODate);

        QString compoundId = dest + id;

        QString finalId = vehicleId.isEmpty()?compoundId:vehicleId;

        Vehicle& v = _trains[!bMinimize?stationName:"" + finalId+lineId];
        if( v.timeToStation == 0 || timeToStation < v.timeToStation || !bMinimize)
        {
            v.vehicleId = vehicleId;
            v.currentLocation = currentLocation;
            v.originalLocation = currentLocation;
            v.towards = towards;
            v.timeToStation = timeToStation;
            v.stationName = stationName;
            v.destinationName = dest;
            v.platformName = platform;
            v.expectedArrival = expectedArrival.toLocalTime().time();
            v.direction = direction;
            v.naptanId = naptanId;
            v.lineId = lineId;
            v._modeName = obj["modeName"].toString();
            v.formatCurrentLocation();
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
    textBrowser -> moveCursor (QTextCursor::Start) ;
    textBrowser -> ensureCursorVisible() ;
}

void Widget::updateTextBrowserWithStations(QTextBrowser *textBrowser)
{
    textBrowser->clear();

    for(const auto& item : _stations)
        textBrowser->append(item.toString());

    textBrowser->append(QString("COUNT:%1").arg(_stations.size()));

    textBrowser -> moveCursor (QTextCursor::Start) ;
    textBrowser -> ensureCursorVisible() ;
}

void Widget::parseNetworkRail(const QJsonDocument &doc)
{
    QJsonArray a = doc.array();

    QTime now = QTime::currentTime();

    for(const QJsonValue &item : std::as_const(a))
    {
        QJsonObject obj = item.toObject();
        QJsonObject header = obj["header"].toObject();
        QJsonObject body = obj["body"].toObject();

        if( header["msg_type"].toString().toInt() != 3)
            continue;

        ui->textBrowser_NetworkRail->append( "event_type:" + body["event_type"].toString());
        ui->textBrowser_NetworkRail->append( "train_id:" + body["train_id"].toString());


        QDateTime plannedTime = QDateTime::fromMSecsSinceEpoch(body["planned_timestamp"].toVariant().toLongLong());

        QDateTime localDateTime = plannedTime.toLocalTime();

        ui->textBrowser_NetworkRail->append( "PlannedTime : " + plannedTime.toString());
        ui->textBrowser_NetworkRail->append( "LocalTime : " + localDateTime.toString());

        QString serviceCode = body["train_service_code"].toString();
        QString toc_id = body["toc_id"].toString();

        const QString direction = body["direction_ind"].toString();

        QString nextStanox = body["next_report_stanox"].toString();

        auto [serviceStanox, timeDiff] = _networkRailScheduleJSON.getDestination(toc_id,
                                                                        serviceCode,
                                                                        body["loc_stanox"].toString(),
                                                                        body["event_type"].toString(),
                                                                        localDateTime.time());

        QString destination = _networkRailStnCSV[serviceStanox].location;

        ui->textBrowser_NetworkRail->append( "train_service_code:" + serviceCode);

        ui->textBrowser_NetworkRail->append( QString("TO:%1 (%2)").arg(destination).arg(timeDiff));


        ui->textBrowser_NetworkRail->append( "platform:" + body["platform"].toString());
        ui->textBrowser_NetworkRail->append( "next_report_run_time:" + body["next_report_run_time"].toString());
        ui->textBrowser_NetworkRail->append( "reporting_stanox:" + _networkRailStnCSV[body["reporting_stanox"].toString()].location);
        ui->textBrowser_NetworkRail->append( "loc_stanox:" + _networkRailStnCSV[body["loc_stanox"].toString()].location);
        ui->textBrowser_NetworkRail->append( "next_report_stanox:" + _networkRailStnCSV[body["next_report_stanox"].toString()].location);
        ui->textBrowser_NetworkRail->append( "direction_ind:" + body["direction_ind"].toString());

        ui->textBrowser_NetworkRail->append("=======================================");
    }
//    ui->textBrowser_NetworkRail->append(str);

}

void Widget::closeEvent(QCloseEvent *event)
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

    s.beginGroup( "mainwindow" );

    s.setValue( "geometry", saveGeometry() );
    s.setValue( "maximized", isMaximized() );
    if ( !isMaximized() )
    {
        s.setValue( "pos", pos() );
        s.setValue( "size", size() );
    }

    s.endGroup();

    _osmTileGenerator->unSetup();
}

void Widget::startTFLRoutesDownload()
{
    ui->pushButtonTFLDownload->setEnabled(false);
    ui->pushButtonStopPoint->setEnabled(false);
    _tflWorker->downloadAllRoutesList(ui->checkBoxInBound->isChecked(), ui->checkBoxAllRoutesBusOnly->isChecked() );
}

void Widget::startTFLStopPointDownload()
{
    ui->pushButtonTFLDownload->setEnabled(false);
    ui->pushButtonStopPoint->setEnabled(false);
    _tflWorker->downloadAllStopPoints();
}

void Widget::processUSStates()
{
    QString str;
    QFile file("data/USAStates/gz_2010_us_040_00_20m.json");

    if( !file.open(QIODevice::ReadOnly))
    {
        qDebug() << file.fileName() << " -- NOT FOUND!";
        return;
    }

    str = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(str.toLatin1());

    QJsonObject rootObj = doc.object();
    QJsonArray states = rootObj["features"].toArray();

    QFile outFile("data/USAStates/USAStates.out");
    qDebug() << "Out file : " << outFile.open(QIODevice::WriteOnly);
    QTextStream stream(&outFile);
    const int jumpSize = 10;

    for( const QJsonValue &value : std::as_const(states))
    {
        QJsonObject geometry = value["geometry"].toObject();
        QJsonArray coords = geometry["coordinates"].toArray();

        for(const QJsonValue &value : std::as_const(coords))
        {
            QJsonArray polygon = value.toArray();
            int outCount = 0;

            for(const QJsonValue &value:std::as_const(polygon))
            {
                QJsonArray latLngGroup = value.toArray();
                if( latLngGroup.size() == 2)
                {
                    double lng = latLngGroup.at(0).toDouble();
                    double lat = latLngGroup.at(1).toDouble();

                    stream << QString("%1+%2\r\n").arg(lat).arg(lng);
                    outCount++;
                    if( outCount % jumpSize ==0)
                    {
                        stream << "-1\r\n";
                        stream << QString("%1+%2\r\n").arg(lat).arg(lng);
                    }
                    continue;
                }

                int count = 0;
                for(const QJsonValue& value : std::as_const(latLngGroup))
                {
                    QJsonArray latLng = value.toArray();
                    double lng = latLng.at(0).toDouble();
                    double lat = latLng.at(1).toDouble();

                    stream << QString("%1+%2\r\n").arg(lat).arg(lng);
                    count++;
                    if( count % jumpSize == 0)
                    {
                        stream << "-1\r\n";
                        stream << QString("%1+%2\r\n").arg(lat).arg(lng);
                    }
                }
                stream << "-1\r\n";
            }
            if( outCount > 0)
                stream << "-1\r\n";
        }
        stream << "-1\r\n";
    }
}

void Widget::processAircraftJson()
{
    qDebug() << Q_FUNC_INFO;

    QString str;
    QFile file("/Project/readsb/webapp/src/db/aircrafts.json");

    if( !file.open(QIODevice::ReadOnly))
    {
        qDebug() << file.fileName() << " -- NOT FOUND!";
        return;
    }

    str = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(str.toLatin1());

    QJsonObject rootObj = doc.object();

    QStringList keys = rootObj.keys();

    QFile outputFile("/Project/readsb/webapp/src/db/helicopters.txt");
    if( !outputFile.open(QIODevice::WriteOnly))
    {
        qDebug() << outputFile.fileName() <<"-- FAILED TO WRITE!";
        return;
    }

    QFile outputFileAircraft("/Project/readsb/webapp/src/db/aircraft_types.txt");
    if( !outputFileAircraft.open(QIODevice::WriteOnly))
    {
        qDebug() << outputFileAircraft.fileName() <<"-- FAILED TO WRITE!";
        return;
    }

    QTextStream streamOutAircraft(&outputFileAircraft);
    QTextStream streamOut(&outputFile);
    int count(0);

    std::set<QString> heliSet;

    for(const QString& key : keys)
    {
        QJsonValue value = rootObj[key];
        QString description = value["d"].toString();
        if( description.contains(QStringLiteral("helicopter"), Qt::CaseInsensitive))
        {
            streamOut << key << "," << value["t"].toString() <<"," << value["r"].toString() << "," << description << "\n";
            heliSet.insert(value["t"].toString());
            qDebug() << key <<"," << value.toString();
        }

        if( !description.isEmpty())
            streamOutAircraft << key << "," << value["t"].toString() <<"," << value["r"].toString() <<"," << description << "\n";

        count++;
        if( count % 1000 == 0)
            qDebug() << count << ":" << keys.count();

    }

    QFile outputFileType("/Project/readsb/webapp/src/db/helicopter_types.txt");
    if( !outputFileType.open(QIODevice::WriteOnly))
    {
        qDebug() << outputFileType.fileName() <<"-- FAILED TO WRITE!";
        return;
    }

    QTextStream streamOutType(&outputFileType);
    for(const QString& type : heliSet)
        streamOutType << type << "\n";
}

void Widget::processOperatorJson()
{
    qDebug() << Q_FUNC_INFO;

    QString str;
    QFile file("/Project/readsb/webapp/src/db/operators.json");

    if( !file.open(QIODevice::ReadOnly))
    {
        qDebug() << file.fileName() << " -- NOT FOUND!";
        return;
    }

    str = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(str.toLatin1());

    QJsonObject rootObj = doc.object();

    QStringList keys = rootObj.keys();

    QFile outputFile("/Project/readsb/webapp/src/db/operators.txt");
    if( !outputFile.open(QIODevice::WriteOnly))
    {
        qDebug() << outputFile.fileName() <<"-- FAILED TO WRITE!";
        return;
    }

    QTextStream streamOut(&outputFile);
    int count(0);

    for(const QString& key : std::as_const(keys))
    {
        QJsonValue value = rootObj[key];

        streamOut << key << "," << value["n"].toString() <<"," << value["c"].toString() << "\n";

        count++;
        if( count % 1000 == 0)
            qDebug() << count << ":" << keys.count();
    }
}

void Widget::runwayBlend()
{
    QImage imgIn;
    imgIn.load("C:/Project/GIT/TFLTest/images/runway.png");

    QImage refImg;
    refImg.load("C:/Project/GIT/TFLTest/images/reference_runway.png");

    for(int y=0; y < imgIn.height(); ++y)
    {
        for(int x =0; x < imgIn.width(); ++x)
        {
            QRgb rgb = imgIn.pixel(x,y);

            if( qRed(rgb) < 200 && qBlue(rgb) < 200 && qGreen(rgb) < 200)
            {
                //imgIn.setPixelColor(x,y, refImg.pixel(float(x)/imgIn.width()* refImg.width(), rand()%refImg.height()));
                imgIn.setPixelColor(x,y, refImg.pixel(rand()% refImg.width(), rand()%refImg.height()));
            }
        }
    }

    imgIn.save("C:/Project/GIT/TFLTest/images/runway_out.png");

}

