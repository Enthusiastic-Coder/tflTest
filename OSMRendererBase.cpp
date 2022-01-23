#include "OSMRendererBase.h"
#include "GPSLocation.h"
#include "OSMData.h"
#include <QDebug>
#include "TFLOSMRenderer.h"

OSMRendererBase::OSMRendererBase(TFLOSMRenderer* view, const WAYPOINTS &wayPoints) :
    _view(view), _wayPoints(wayPoints)
{

}

OSMRendererBase::~OSMRendererBase()
{
}

void OSMRendererBase::clear()
{
    _osmPts.clear();
    _osmTagCache.clear();
}

void OSMRendererBase::paint(QPainter &p)
{
    QPen oldPen = p.pen();

    const float pixM = _view->getPixelLevel();

    QPen pen(_view->isMapNight()? _nightColor: _dayColor);
    pen.setWidthF(qMax(_lineThickness, _lineThickness* pixM / 1000.0));
    p.setPen( pen);

    for( const auto& pts : qAsConst(_osmPts))
        p.drawPolyline(pts);

    p.setPen(oldPen);
}

void OSMRendererBase::paintText(QPainter &p)
{
    if( !_isVisible)
        return;

    const auto& osmTagCache = _osmTagCache;

    QFont labelFont;
    labelFont.setFamily("Verdana");
    labelFont.setPixelSize(_view->getPixelLevel() > 2000? 14: 10);
    p.setFont(labelFont);

    p.setPen(_view->isMapNight()? Qt::white:Qt::darkGray);

    QFontMetrics fm = p.fontMetrics();

    for(const auto& tagItem : osmTagCache)
    {
        if( fm.horizontalAdvance(tagItem.second->tags[0].second) > tagItem.first.second)
            continue;

        const QString& textToRender = tagItem.second->tags[0].second;

        if( tagItem.second->bearings.empty())
        {
            if( _view->ptInScreen(tagItem.first.first))
                p.drawText(tagItem.first.first, textToRender);
        }
        else
        {
            QTransform oldT = p.transform();
            p.translate(tagItem.first.first);
            float brg = tagItem.second->bearings[tagItem.second->bearings.size()/2];

            bool flip = MathSupport<float>::normAng(brg -  _view->getCompassValue()) > 179.0f;
            if( flip )
                p.rotate(int(brg+90));
            else
                p.rotate(int(brg-90));

            const int textWidth = fm.horizontalAdvance(textToRender);

            QRect rc;
            rc.setWidth(textWidth);
            rc.setHeight(fm.height());
            rc.setY(-rc.height());

            if( !flip)
                rc.setX(-textWidth);

            if( _view->ptInScreen(QPoint(rc.left(), rc.top())))
                p.drawText(rc, textToRender);

            if( !tagItem.second->bearings.empty())
                p.setTransform(oldT);
        }
    }
}

void OSMRendererBase::updateCache()
{
    if( !_isVisible)
        return;

    auto& osmPts = _osmPts;
    auto& osmTagCache = _osmTagCache;

    osmPts.clear();
    osmTagCache.clear();

    GPSLocation lastPos;

    for(const auto& wayPoint: _wayPoints)
    {
        QVector<QPoint> pts;

        if( wayPoint->gpsPts.size() < 2)
            continue;

        const GPSLocation& p1 = wayPoint->gpsPts[0];
        const GPSLocation& p2 = wayPoint->gpsPts[wayPoint->gpsPts.size()-1];

        QPoint a(_view->toScreen(p1));
        QPoint b(_view->toScreen(p2));

        if( !_view->ptInScreen(p1) && !_view->ptInScreen(p2))
            continue;

        for(const auto& gpsPt : wayPoint->gpsPts)
        {
            if( !_view->ptInScreen(gpsPt))
                continue;

            pts << _view->toScreen(gpsPt);
        }

        if( _view->getPixelLevel() > 500 )
            if( wayPoint->tags.size() > 0 && wayPoint->tags.size() > 1)
                osmTagCache.push_back(std::make_pair(std::make_pair(pts[pts.size()/2], (a-b).manhattanLength()), wayPoint.get()));

        osmPts << pts;

    }

#ifdef Q_OS_WIN
//    qDebug() << objectName() << ":" << itemCount << " - " << skippedCount;
#endif
}

float OSMRendererBase::getPixelsPerMile() const
{
    return _pixelPerMile;
}

void OSMRendererBase::setVisible(bool b)
{
    _isVisible = b;
}

bool OSMRendererBase::isVisible() const
{
    return _isVisible;
}

void OSMRenderMotorWay::init()
{
    _dayColor = QColor("#cccc00");
    _nightColor = Qt::darkYellow;
    _lineThickness = 5.0;
    _pixelPerMile = 0.0f;
}

void OSMRenderPrimary::init()
{
    _dayColor = Qt::white;
    _nightColor = Qt::darkGray;
    _lineThickness = 5.0;
    _pixelPerMile = 100.0f;
}

void OSMRenderSecondary::init()
{
    _dayColor = Qt::white;
    _nightColor = Qt::lightGray;
    _lineThickness = 3.0;
    _pixelPerMile = 300.0f;
}

void OSMRenderTertiary::init()
{
    _dayColor = Qt::white;
    _nightColor = Qt::lightGray;
    _lineThickness = 3.0;
    _pixelPerMile = 300.0f;
}

void OSMRenderResidential::init()
{
    _dayColor = Qt::white;
    _nightColor = Qt::lightGray;
    _lineThickness = 2.0;
    _pixelPerMile = 300.0f;
}

void OSMRenderFootway::init()
{
    _dayColor = Qt::lightGray;
    _nightColor = Qt::darkGray;
    _lineThickness = 1.0;
    _pixelPerMile = 300.0f;
}

void OSMRenderWater::init()
{
    _dayColor = Qt::blue;
    _nightColor = Qt::darkCyan;
    _lineThickness = 1.0;
    _pixelPerMile = 300.0f;
}

void OSMRenderAeroWay::init()
{
    _dayColor = Qt::lightGray;
    _nightColor = Qt::lightGray;
    _lineThickness = 2.0;
    _pixelPerMile = 100.0f;
}

void OSMRenderAeroRunway::init()
{
    _dayColor = Qt::lightGray;
    _nightColor = Qt::white;
    _lineThickness = 15.0;
    _pixelPerMile = 100.0f;
}

void OSMRenderCycleWay::init()
{
    _dayColor = Qt::lightGray;
    _nightColor = Qt::darkGray;
    _lineThickness = 1.0;
    _pixelPerMile = 300.0f;
}

void OSMRenderPedestrian::init()
{
    _dayColor = Qt::white;
    _nightColor = Qt::lightGray;
    _lineThickness = 2.0;
    _pixelPerMile = 300.0f;
}
