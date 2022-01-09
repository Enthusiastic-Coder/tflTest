#include "OSMTileGenerator.h"
#include "ui_Widget.h"
#include <QSettings>

OSMTileGenerator::OSMTileGenerator(QObject *parent)
    : QObject(parent)
{

}

OSMTileGenerator::~OSMTileGenerator()
{
}

void OSMTileGenerator::setUp(Ui::Widget *ui)
{
    connect(ui->pushButtonGenerateOSMTile, &QPushButton::clicked, this, [this] {

    });


    QSettings s;

}

void OSMTileGenerator::unSetup()
{
    QSettings s;

}
