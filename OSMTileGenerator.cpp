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
        {
            (_data.*pFunc)(filename);
            _ui->textEditOSMLoadDebug->append(filename + " imported.");
        }
    };

    connect(ui->pushButtonLoadOSMData, &QPushButton::clicked, this, [this,importOSM] {

        importOSM(&OSMData::importMotorway, _ui->lineEditOSMMotorwayPath->text());
        importOSM(&OSMData::importPrimary, _ui->lineEditOSMPrimaryPath->text());
    });

    _settingsMappings.push_back({"OSMMotorwayPath", _ui->lineEditOSMMotorwayPath});
    _settingsMappings.push_back({"OSMPrimaryPath", _ui->lineEditOSMPrimaryPath});

    QSettings s;

    for(auto& item : _settingsMappings)
        item.edit->setText(s.value(item.name).toString());
}

void OSMTileGenerator::unSetup()
{
    QSettings s;

    for(auto& item : _settingsMappings)
        s.setValue(item.name, item.edit->text());
}
