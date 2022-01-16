#ifndef OSMTILEGENERATOR_H
#define OSMTILEGENERATOR_H

#include <QObject>
#include "OSMData.h"

namespace Ui {
class Widget;
}

class QLineEdit;

class OSMTileGenerator : public QObject
{
    Q_OBJECT
public:
    using SettingEntry = struct {
        QString name;
        QLineEdit* edit;
    };

    OSMTileGenerator(QObject* parent);
    ~OSMTileGenerator();

    void setUp(Ui::Widget *ui);
    void unSetup();

private slots:
    void generateTiles();

private:
    void addLog(const QString &line);


private:
    Ui::Widget* _ui;
    OSMData _data;
    std::vector<SettingEntry> _settingsMappings;
};

#endif // OSMTILEGENERATOR_H
