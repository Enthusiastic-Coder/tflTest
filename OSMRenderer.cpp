#include <jibbs/math/MathSupport.h>
#include <jibbs/vector/vector3.h>

#include <QSettings>

#include "OSMData.h"
#include "OSMRenderer.h"

#define RENDER_TYPE(name, obj) _renderObjects.push_back(obj = new OSMRender##name(this, _osmData->get##name()));

OSMRenderer::OSMRenderer(OSMData *osmData):
    _osmData(osmData)
{
}

void OSMRenderer::init()
{
    RENDER_TYPE(AeroWay, _aeroway);
    RENDER_TYPE(AeroRunway, _aerorunway);
    RENDER_TYPE(Water, _water);
    RENDER_TYPE(Footway, _footway);
    RENDER_TYPE(CycleWay, _cycleWay);
    RENDER_TYPE(Pedestrian, _pedestrian);
    RENDER_TYPE(Tertiary, _tertiary);
    RENDER_TYPE(Residential, _residential);
    RENDER_TYPE(Secondary, _secondary);
    RENDER_TYPE(Primary, _primary);
    RENDER_TYPE(MotorWay, _motorway);

    for( auto& renderObject : _renderObjects)
    {
        renderObject->init();
        GPSLocation topLeft, bottomRight;
        renderObject->calcBoundingBox(topLeft, bottomRight);
        calcBoundingBox(topLeft, bottomRight);
    }
}

void OSMRenderer::updateCache()
{
    perform(nullptr);
}

void OSMRenderer::paint(QPainter &p)
{
    perform(&p);
}

bool OSMRenderer::isEmpty() const
{
    for( auto& renderObject : _renderObjects)
        if( !renderObject->isEmpty())
            return false;

    return true;
}

void OSMRenderer::paintText(QPainter &p)
{
    for( auto& renderObject : _renderObjects)
        renderObject->paintText(p);
}

void OSMRenderer::setZoomLevel(float p)
{
    _zoomLevel = p;
}

int OSMRenderer::getZoomLevel() const
{
    return _zoomLevel;
}

void OSMRenderer::setMapNight(bool n)
{
    _isNight = n;
}

bool OSMRenderer::isMapNight() const
{
    return _isNight;
}

GPSLocation OSMRenderer::topLeft() const
{
    return _topLeft;
}

GPSLocation OSMRenderer::bottomRight() const
{
    return _bottomRight;
}

void OSMRenderer::setScaleFactor(int factor)
{
    _scaleFactor = factor;
}

void OSMRenderer::setSize(QSize sz)
{
    _size = sz;
}

QSize OSMRenderer::size() const
{
    return _size;
}

QSize OSMRenderer::imageSize() const
{
    return QSize(_size.width()*_scaleFactor, _size.height()* _scaleFactor);
}

float OSMRenderer::getCompassValue() const
{
    return 0.0f;
}

int OSMRenderer::getTileHorizontals() const
{
    GPSLocation tl = topLeft();
    GPSLocation tr = bottomRight();
    tr._lat = tl._lat;

    float distance = tl.distanceTo(tr)/1609.34f;

    int pixels = distance * _zoomLevel;

    return std::max(1,pixels/_size.width()+1);
}

int OSMRenderer::getTileVerticals() const
{
    GPSLocation tl = topLeft();
    GPSLocation tr = bottomRight();

    tr._lng = tl._lng;

    float distance = tl.distanceTo(tr)/1609.34f;
    int pixels = distance * _zoomLevel;

    return std::max(1,pixels/_size.height()+1);
}

void OSMRenderer::setLocation(const GPSLocation &l)
{
    _location = l;
}

GPSLocation OSMRenderer::getLocation() const
{
    return _location;
}

void OSMRenderer::setTileIndex(int xIndex, int yIndex)
{
    const int xCount = getTileHorizontals();
    const int yCount = getTileVerticals();

    GPSLocation tileDiff = bottomRight() - topLeft();

    double lat = float(yIndex)/yCount * tileDiff._lat + topLeft()._lat;
    double lng =  float(xIndex)/xCount * tileDiff._lng + topLeft()._lng;

    GPSLocation location(lat,lng);

    location._lat += tileDiff._lat/yCount/2;
    location._lng += tileDiff._lng/xCount/2;

    setLocation(location);
}

QPoint OSMRenderer::toScreen(const GPSLocation& l) const
{
    const int xCount = getTileHorizontals();
    const int yCount = getTileVerticals();

    GPSLocation tileDiff = bottomRight() - topLeft();

    double pixPerLat = _size.height()/ (tileDiff._lat/yCount);
    double pixPerLng = _size.width() / (tileDiff._lng/xCount);

    GPSLocation diff = l - _location;

    int dPy = diff._lat * pixPerLat;
    int dPx = diff._lng * pixPerLng;

    return QPoint((_size.width()/2 + dPx)*_scaleFactor, (_size.height()/2 + dPy)*_scaleFactor);
}

bool OSMRenderer::ptInScreen(QPoint pt) const
{
    int border = 200;
    return pt.x() > -border && pt.x() < _size.width()*_scaleFactor+border
            && pt.y() >-border && pt.y() < _size.height()*_scaleFactor+border;
}

bool OSMRenderer::ptInScreen(const GPSLocation& l) const
{
    return ptInScreen(toScreen(l));
}

void OSMRenderer::perform(QPainter* p)
{
    for( auto& renderObject : _renderObjects)
    {
        if( p == nullptr)
            renderObject->clear();

        if( getZoomLevel() >= renderObject->getZoomLevel())
        {
            if( p == nullptr)
                renderObject->updateCache();
            else
                renderObject->paint(*p);
        }
    }
}

void OSMRenderer::calcBoundingBox(const GPSLocation &topLeft, const GPSLocation &bottomRight)
{
    bool topLeftStarted = false;
    bool bottomLeftStarted = false;

    std::vector<GPSLocation> gpsPts = {topLeft, bottomRight};

    for( auto& gpsPt : gpsPts)
    {
        if( !topLeftStarted && _topLeft == GPSLocation())
        {
            topLeftStarted = true;
            _topLeft = gpsPt;
        }
        else
        {
            _topLeft._lat = std::max( _topLeft._lat, gpsPt._lat);
            _topLeft._lng = std::min( _topLeft._lng, gpsPt._lng);
        }

        if( !bottomLeftStarted && _bottomRight == GPSLocation())
        {
            bottomLeftStarted = true;
            _bottomRight = gpsPt;
        }
        else
        {
            _bottomRight._lat = std::min( _bottomRight._lat, gpsPt._lat);
            _bottomRight._lng = std::max( _bottomRight._lng, gpsPt._lng);
        }
    }
}
