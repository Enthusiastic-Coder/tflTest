#include "OSMTileGenerator.h"

#include "ui_Widget.h"


OSMTileGenerator::OSMTileGenerator(QObject *parent)
    : QObject(parent)
{

}

void OSMTileGenerator::setUp(Ui::Widget *ui)
{
    connect(ui->pushButtonGenerateOSMTile, &QPushButton::clicked, [this] {

    });

}
