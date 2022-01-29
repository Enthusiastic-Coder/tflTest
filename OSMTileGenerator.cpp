#include "OSMTileGenerator.h"
#include "ui_Widget.h"
#include <QSettings>
#include "OSMData.h"
#include <QDoubleValidator>
#include "TFLOSMRenderer.h"
#include <QDir>
#include <QTime>

OSMTileGenerator::OSMTileGenerator(QObject *parent)
    : QObject(parent)
{

}

OSMTileGenerator::~OSMTileGenerator()
{
}

void OSMTileGenerator::setUp(Ui::Widget *ui)
{
    _ui = ui;

    ui->lineEditOSMZoomLevel->setValidator( new QDoubleValidator(this) );

    connect(ui->pushButtonGenerateOSMTile, &QPushButton::clicked, this, &OSMTileGenerator::generateTiles);

    auto importOSM = [this] ( void (OSMData::*pFunc)(const QString&), QString filename) {

        if( !filename.isEmpty())
        {
            (_data.*pFunc)(filename);
            addLog(filename + " imported.");
        }
    };

    connect(ui->pushButtonLoadOSMData, &QPushButton::clicked, this, [this,importOSM] {

        addLog("ImportOSM: BEGIN");
        importOSM(&OSMData::importMotorWay, _ui->lineEditOSMMotorwayPath->text());
        importOSM(&OSMData::importPrimary, _ui->lineEditOSMPrimaryPath->text());
        importOSM(&OSMData::importSecondary, _ui->lineEditOSMSecondaryPath->text());
        importOSM(&OSMData::importTertiary, _ui->lineEditOSMTertiaryPath->text());
        importOSM(&OSMData::importResidential, _ui->lineEditOSMResidentialPath->text());
        importOSM(&OSMData::importAeroRunway, _ui->lineEditOSMAerorunwayPath->text());
        importOSM(&OSMData::importAeroWay, _ui->lineEditOSMAerowayPath->text());
        importOSM(&OSMData::importPedestrian, _ui->lineEditOSMPedestrianPath->text());
        importOSM(&OSMData::importFootway, _ui->lineEditOSMFootwayPath->text());
        importOSM(&OSMData::importCycleWay, _ui->lineEditOSMCyclewayPath->text());
        importOSM(&OSMData::importWater, _ui->lineEditOSMWaterPath->text());
        addLog("ImportOSM: END");
    });

    _osmSettings.push_back({"OSMMotorwayPath", _ui->lineEditOSMMotorwayPath});
    _osmSettings.push_back({"OSMPrimaryPath", _ui->lineEditOSMPrimaryPath});
    _osmSettings.push_back({"OSMSecondaryPath", _ui->lineEditOSMSecondaryPath});
    _osmSettings.push_back({"OSMTertiaryPath", _ui->lineEditOSMTertiaryPath});
    _osmSettings.push_back({"OSMResidentialPath", _ui->lineEditOSMResidentialPath});
    _osmSettings.push_back({"OSMAeroRunwayPath", _ui->lineEditOSMAerorunwayPath});
    _osmSettings.push_back({"OSMAerowayPath", _ui->lineEditOSMAerowayPath});
    _osmSettings.push_back({"OSMPedestrianPath", _ui->lineEditOSMPedestrianPath});
    _osmSettings.push_back({"OSMPFootwayPath", _ui->lineEditOSMFootwayPath});
    _osmSettings.push_back({"OSMPCyclePath", _ui->lineEditOSMCyclewayPath});
    _osmSettings.push_back({"OSMPWaterPath", _ui->lineEditOSMWaterPath});

    QSettings s;

    for(auto& item : _osmSettings)
        item.edit->setText(s.value(item.name).toString());

    ui->lineEditOSMZoomLevel->setText(s.value("OSMZoomLevel").toString());
    ui->checkBoxOSMNightTime->setChecked(s.value("OSMNightTime").toBool());
    ui->lineEditOSMOutputPath->setText(s.value("OSMOutputPath").toString());
}

void OSMTileGenerator::unSetup()
{
    QSettings s;

    for(auto& item : _osmSettings)
        s.setValue(item.name, item.edit->text());

    s.setValue("OSMZoomLevel", _ui->lineEditOSMZoomLevel->text());
    s.setValue("OSMTightTime", _ui->checkBoxOSMNightTime->isChecked() );
    s.setValue("OSMOutputPath",_ui->lineEditOSMOutputPath->text());
}

void OSMTileGenerator::generateTiles()
{
    addLog("GenerateTiles: BEGIN");

    QStringList zoomLevels = _ui->lineEditOSMZoomLevel->text().split(",");

    if(zoomLevels.empty())
    {
        addLog("GenerateTiles: No Zoom levels found");
        return;
    }

    const QString outputPathStr = _ui->lineEditOSMOutputPath->text();

    if(outputPathStr.isEmpty())
    {
        addLog("GenerateTiles: No output path set");
        return;
    }

    {
        QDir outpath(outputPathStr);
        if( !outpath.exists())
        {
            addLog("GenerateTiles: Output path does NOT exist");
            return;
        }
    }

    QSize sz(1024,1024);

    std::unique_ptr<TFLOSMRenderer> renderer = std::make_unique<TFLOSMRenderer>(&_data);

    renderer->init();
    renderer->setMapNight(_ui->checkBoxOSMNightTime->isChecked());
    renderer->setSize(sz);

    addLog("BoundingBox:" + QString::fromStdString(renderer->topLeft().toString()) + "==" + QString::fromStdString(renderer->bottomRight().toString()));

    for(auto& zoomLevel: zoomLevels)
    {
        renderer->setPixelLevel(zoomLevel.toFloat());
        renderer->setLocation(GPSLocation(51.4964, -0.300198));

        addLog("Output: TileHorz:" + QString::number(renderer->getTileHorizontals()));
        addLog("Output: TileVert:" + QString::number(renderer->getTileVerticals()));

        QImage image(sz,QImage::Format_ARGB32);
        image.fill(    renderer->isMapNight()? QColor::fromRgbF(0.1f,0.1f,0.1f):
                                               QColor::fromRgbF(0.85f,0.85f,0.85f));

        QPainter p(&image);

        renderer->updateCache();
        renderer->paint(p);

        QDir outpath(outputPathStr);

        outpath.mkdir(zoomLevel);
        outpath.cd(zoomLevel);
        QString outfilename = outpath.filePath("OSM_TILE.png");

        image.save(outfilename);

        addLog("Output:" + outfilename);
    }
    addLog("GenerateTiles: END");
}

void OSMTileGenerator::addLog(const QString &line)
{
    _ui->textEditOSMLoadDebug->append(QTime::currentTime().toString() + ":" + line);

}
