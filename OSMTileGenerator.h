#ifndef OSMTILEGENERATOR_H
#define OSMTILEGENERATOR_H

#include <QObject>

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
};

#endif // OSMTILEGENERATOR_H
