#include "OSMTileGenerator.h"
#include "ui_Widget.h"
#include <QSettings>
#include "OSMData.h"

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

    connect(ui->pushButtonGenerateOSMTile, &QPushButton::clicked, this, [this] {

    });

    auto importOSM = [this] ( void (OSMData::*pFunc)(const QString&), QString filename) {

        if( !filename.isEmpty())
            (_data.*pFunc)(filename);
    };

    connect(ui->pushButtonLoadOSMData, &QPushButton::clicked, this, [this,importOSM] {

        importOSM(&OSMData::importMotorway, _ui->lineEditOSMMotorwayPath->text());
    });


    QSettings s;
    _ui->lineEditOSMMotorwayPath->setText(s.value("OSMMotorwayPath").toString());
}

void OSMTileGenerator::unSetup()
{
    QSettings s;

    s.setValue("OSMMotorwayPath", _ui->lineEditOSMMotorwayPath->text());
}
