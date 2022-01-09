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

    void setUp(Ui::Widget *ui);
};

#endif // OSMTILEGENERATOR_H
