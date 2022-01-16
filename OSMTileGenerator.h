#ifndef OSMTILEGENERATOR_H
#define OSMTILEGENERATOR_H

#include <QObject>
#include "OSMData.h"

namespace Ui {
class Widget;
}

class OSMTileGenerator : public QObject
{
    Q_OBJECT
public:
    OSMTileGenerator(QObject* parent);
    ~OSMTileGenerator();

    void setUp(Ui::Widget *ui);
    void unSetup();

private:
    Ui::Widget* _ui;
    OSMData _data;
};

#endif // OSMTILEGENERATOR_H
