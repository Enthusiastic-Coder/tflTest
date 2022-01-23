#include "OSMTileGenerator.h"
#include "ui_Widget.h"
#include <QSettings>
#include "OSMData.h"
#include <QDoubleValidator>
#include "TFLOSMRenderer.h"

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

        importOSM(&OSMData::importMotorWay, _ui->lineEditOSMMotorwayPath->text());
        importOSM(&OSMData::importPrimary, _ui->lineEditOSMPrimaryPath->text());
    });

    _osmSettings.push_back({"OSMMotorwayPath", _ui->lineEditOSMMotorwayPath});
    _osmSettings.push_back({"OSMPrimaryPath", _ui->lineEditOSMPrimaryPath});

    QSettings s;

    for(auto& item : _osmSettings)
        item.edit->setText(s.value(item.name).toString());
}

void OSMTileGenerator::unSetup()
{
    QSettings s;

    for(auto& item : _osmSettings)
        s.setValue(item.name, item.edit->text());
}

void OSMTileGenerator::generateTiles()
{
    addLog("GenerateTiles:");
    QSize sz(128,128);

    std::unique_ptr<TFLOSMRenderer> renderer = std::make_unique<TFLOSMRenderer>(&_data);

    renderer->init();

    renderer->setPixelLevel(_ui->lineEditOSMZoomLevel->text().toFloat());
    renderer->setMapNight(false);
    renderer->setLocation(GPSLocation(51.4964, -0.300198, 392.0));
    renderer->setSize(sz);

    QImage image(sz,QImage::Format_ARGB32);
    QPainter p(&image);

    renderer->updateCache();
    renderer->paint(p);

    image.save("OSM_TILE.png");
}

void OSMTileGenerator::addLog(const QString &line)
{
    _ui->textEditOSMLoadDebug->append(line);

}
