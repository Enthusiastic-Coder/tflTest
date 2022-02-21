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

    connect(ui->pushButtonGenerateOSMTile, &QPushButton::clicked, this, [this] {
        generateTiles(false);
    });

    connect(ui->pushButtonGenerateOSMSingleTile, &QPushButton::clicked, this, [this] {
        generateTiles(true);
    });

    connect(ui->pushButtonOSMInfo, &QPushButton::clicked, this, [this] {

        QSize sz(1024,1024);

        std::unique_ptr<TFLOSMRenderer> renderer = std::make_unique<TFLOSMRenderer>(&_data);

        renderer->init();
        renderer->setMapNight(_ui->checkBoxOSMNightTime->isChecked());
        renderer->setSize(sz);

        addLog("BoundingBox:" + QString::fromStdString(renderer->topLeft().toString()) + "==" + QString::fromStdString(renderer->bottomRight().toString()));

        QStringList zoomLevels = _ui->lineEditOSMZoomLevel->text().split(",");

        if(!zoomLevels.empty())
        {
            for(auto& zoomLevel: zoomLevels)
            {
                renderer->setPixelLevel(zoomLevel.toFloat());
                int cx = renderer->getTileHorizontals();
                int cy = renderer->getTileVerticals();
                addLog(zoomLevel + " -> " + QString::number(cx) +" x " + QString::number(cy) +" = " + QString::number(cx*cy));
            }
        }
    });

    connect(ui->pushButtonOSMLoadDebug, &QPushButton::clicked, ui->textEditOSMLoadDebug, &QTextEdit::clear);

    auto importOSM = [this] ( void (OSMData::*pFunc)(const QString&), QString filename) {

        if( !filename.isEmpty())
        {
            (_data.*pFunc)(filename);
            addLog(filename + " imported.");
        }
    };

    connect(ui->pushButtonLoadOSMData, &QPushButton::clicked, this, [this,importOSM] {

#define Root "c:/project/git/tfltracker/data/OSM/"

        addLog("ImportOSM: BEGIN");
        importOSM(&OSMData::importMotorWay, Root"greater-london-latest_highway_motorway.bin");
        importOSM(&OSMData::importMotorWay, Root"greater-london-latest_highway_trunk.bin");

        importOSM(&OSMData::importPrimary, Root"greater-london-latest_highway_primary.bin");
        importOSM(&OSMData::importSecondary, Root"greater-london-latest_highway_secondary.bin");

        importOSM(&OSMData::importTertiary, Root"greater-london-latest_highway_tertiary.bin");
        importOSM(&OSMData::importTertiary, Root"greater-london-latest_highway_unclassified.bin");
        importOSM(&OSMData::importTertiary, Root"greater-london-latest_highway_service.bin");

        importOSM(&OSMData::importResidential, Root"greater-london-latest_highway_residential.bin");
        importOSM(&OSMData::importAeroRunway, Root"greater-london-latest_aeroway_runway.bin");
        importOSM(&OSMData::importAeroWay, Root"greater-london-latest_aeroway_taxiway.bin");
        importOSM(&OSMData::importPedestrian, Root"greater-london-latest_highway_pedestrian.bin");
        importOSM(&OSMData::importFootway, Root"greater-london-latest_highway_footway.bin");
        importOSM(&OSMData::importCycleWay, Root"greater-london-latest_highway_cycleway.bin");
        importOSM(&OSMData::importWater, Root"london-water.bin");
        addLog("ImportOSM: END");
    });

    QSettings s;

    ui->lineEditOSMZoomLevel->setText(s.value("OSMZoomLevel").toString());
    ui->checkBoxOSMNightTime->setChecked(s.value("OSMNightTime").toBool());
    ui->lineEditOSMOutputPath->setText(s.value("OSMOutputPath").toString());
}

void OSMTileGenerator::unSetup()
{
    QSettings s;

    s.setValue("OSMZoomLevel", _ui->lineEditOSMZoomLevel->text());
    s.setValue("OSMNightTime", _ui->checkBoxOSMNightTime->isChecked() );
    s.setValue("OSMOutputPath",_ui->lineEditOSMOutputPath->text());
}

void OSMTileGenerator::generateTiles(bool bSample)
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

    {
        QFile fileOut(outputPathStr +"/bounds.txt");
        fileOut.open(QIODevice::WriteOnly);
        QTextStream stream(&fileOut);
        stream << QString::fromStdString(renderer->topLeft().toString()) << "\n";
        stream << QString::fromStdString(renderer->bottomRight().toString());
        fileOut.close();
    }

    auto generateTileImage = [this,outputPathStr,sz](std::unique_ptr<TFLOSMRenderer>& renderer, QString zoomLevel, QString outfilename) {

        QImage image(sz,QImage::Format_ARGB32);
        image.fill(    renderer->isMapNight()? QColor::fromRgbF(0.1f,0.1f,0.1f):
                                               QColor::fromRgbF(0.85f,0.85f,0.85f));

        QPainter p(&image);
        renderer->paint(p);
        renderer->paintText(p);

        image.save(outfilename);

        addLog("Output:" + outfilename);
    };

    QDir outpath(outputPathStr);

    for(auto& zoomLevel: zoomLevels)
    {
        renderer->setPixelLevel(zoomLevel.toFloat());

        outpath.mkdir(zoomLevel);
        outpath.cd(zoomLevel);

        {
            QFile fileOut(outpath.absolutePath() +"/dims.txt");
            fileOut.open(QIODevice::WriteOnly);
            QTextStream stream(&fileOut);
            stream << renderer->getTileHorizontals() << "\n";
            stream << renderer->getTileVerticals();
            fileOut.close();
        }

        QString timeofDay = renderer->isMapNight()?"night" :"day";
        outpath.mkdir(timeofDay);
        outpath.cd(timeofDay);

        if(bSample)
        {
            renderer->setLocation(GPSLocation(51.4756, -0.451969));
            renderer->updateCache();
            generateTileImage(renderer, zoomLevel, outpath.absoluteFilePath("Sample.png"));
        }
        else
        {

            addLog("Output: TileHorz:" + QString::number(renderer->getTileHorizontals()));
            addLog("Output: TileVert:" + QString::number(renderer->getTileVerticals()));

            const int xCount = renderer->getTileHorizontals();
            const int yCount = renderer->getTileVerticals();

            for(int y= 0; y < yCount; ++y)
            {
                for( int x=0; x < xCount; ++x)
                {
                    renderer->setTileIndex(x, y);
                    renderer->updateCache();

                    if( renderer->isEmpty())
                        continue;

                    generateTileImage(renderer, zoomLevel, QString("%1/%2_%3.png").arg(outpath.absolutePath()).arg(x).arg(y));

                    addLog("Loc:"+ QString::fromStdString(renderer->getLocation().toString()));

                    QCoreApplication::processEvents();
                }
            }
        }
        addLog("--------------------------");
        outpath.cdUp();
        outpath.cdUp();
    }

    addLog("GenerateTiles: END");
}

void OSMTileGenerator::addLog(const QString &line)
{
    _ui->textEditOSMLoadDebug->append(QTime::currentTime().toString() + ":" + line);

}
