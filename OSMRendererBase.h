#ifndef OSMRENDERERBASE_H
#define OSMRENDERERBASE_H

#include "OSMData.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QPainter>

class TFLOSMRenderer;

class OSMRendererBase : public QObject
{
    Q_OBJECT
public:
    OSMRendererBase(TFLOSMRenderer *view, const WAYPOINTS& wayPoints);
    virtual ~OSMRendererBase();

    virtual void init() = 0;

    void clear();
    void paint(QPainter& p);
    void paintText(QPainter& p);
    void updateCache();

    void calcBoundingBox(GPSLocation& topLeft, GPSLocation& bottomRight);

    float getPixelsPerMile() const;

    void setVisible(bool b);
    bool isVisible() const;

protected:
    TFLOSMRenderer* _view;
    bool _isVisible = true;
    QVector<QVector<QPoint>> _osmPts;
    std::vector<std::pair<std::pair<QPoint,int>,OSM_WAYPOINT*>> _osmTagCache;

    const WAYPOINTS& _wayPoints;
    QColor _dayColor = Qt::white;
    QColor _nightColor = Qt::white;
    qreal _lineThickness = 5.0;
    float _pixelPerMile = 1.0f;
};

#define OSMCLASS(view,name) name(TFLOSMRenderer* view,const WAYPOINTS& wayPoints) \
    : OSMRendererBase(view, wayPoints) {setObjectName(#name);}

#define OSMTYPE(name) \
    class name : public OSMRendererBase\
    {\
    public:\
        OSMCLASS(view, name)\
        void init() override;\
    };

OSMTYPE(OSMRenderMotorWay)
OSMTYPE(OSMRenderPrimary)
OSMTYPE(OSMRenderSecondary)
OSMTYPE(OSMRenderTertiary)
OSMTYPE(OSMRenderResidential)
OSMTYPE(OSMRenderFootway)
OSMTYPE(OSMRenderWater)
OSMTYPE(OSMRenderAeroWay)
OSMTYPE(OSMRenderAeroRunway)
OSMTYPE(OSMRenderCycleWay)
OSMTYPE(OSMRenderPedestrian)

#endif // OSMRENDERERBASE_H
