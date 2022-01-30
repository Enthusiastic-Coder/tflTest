#include "TFLOSMRenderer.h"
#include "OSMData.h"
#include <QSettings>
#include <MathSupport.h>
#include <vector3.h>

#define RENDER_TYPE(name, obj) _renderObjects.push_back(obj = new OSMRender##name(this, _osmData->get##name()));

TFLOSMRenderer::TFLOSMRenderer(OSMData *osmData):
    _osmData(osmData)
{
}

void TFLOSMRenderer::init()
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

void TFLOSMRenderer::updateCache()
{
    perform(nullptr);
}

void TFLOSMRenderer::paint(QPainter &p)
{
    perform(&p);
}

void TFLOSMRenderer::paintText(QPainter &p)
{
    for( auto& renderObject : _renderObjects)
        renderObject->paintText(p);
}

void TFLOSMRenderer::setPixelLevel(float p)
{
    _pixelPerMile = p;
}

float TFLOSMRenderer::getPixelLevel() const
{
    return _pixelPerMile;
}

void TFLOSMRenderer::setMapNight(bool n)
{
    _isNight = n;
}

bool TFLOSMRenderer::isMapNight() const
{
    return _isNight;
}

GPSLocation TFLOSMRenderer::topLeft() const
{
    return _topLeft;
}

GPSLocation TFLOSMRenderer::bottomRight() const
{
    return _bottomRight;
}

void TFLOSMRenderer::setSize(QSize sz)
{
    _size = sz;
}

float TFLOSMRenderer::getCompassValue() const
{
    return 0.0f;
}

int TFLOSMRenderer::getTileHorizontals() const
{
    GPSLocation tl = topLeft();
    GPSLocation tr = bottomRight();
    tr._lat = tl._lat;

    float distance = tl.distanceTo(tr)/1609.34f;

    int pixels = distance * _pixelPerMile;

    return std::max(1,pixels/_size.width()+1);
}

int TFLOSMRenderer::getTileVerticals() const
{
    GPSLocation tl = topLeft();
    GPSLocation tr = bottomRight();

    tr._lng = tl._lng;

    float distance = tl.distanceTo(tr)/1609.34f;
    int pixels = distance * _pixelPerMile;

    return std::max(1,pixels/_size.height()+1);
}

void TFLOSMRenderer::setLocation(const GPSLocation &l)
{
    _location = l;
}

GPSLocation TFLOSMRenderer::getLocation() const
{
    return _location;
}

void TFLOSMRenderer::setTileIndex(int index)
{
    const int xCount = getTileHorizontals();
    const int yCount = getTileVerticals();

    int xIndex = index % xCount;
    int yIndex = index / xCount;

    double dLat = bottomRight()._lat - topLeft()._lat;
    double lat = float(yIndex)/yCount * dLat + topLeft()._lat;

    double dLng = bottomRight()._lng - topLeft()._lng;
    double lng =  float(xIndex)/xCount * dLng + topLeft()._lng;

    GPSLocation location(lat,lng);

//    location._lat += -dLat/yCount/2;
//    location._lng += dLng/xCount/2;

    setLocation(location);
}

QPoint TFLOSMRenderer::toScreen(const GPSLocation& l) const
{
    float brg = l.bearingFrom(_location);
    float dist = l.distanceTo(_location)/1609.334 * _pixelPerMile;

    auto q = MathSupport<float>::MakeQHeading(brg);

    Vector3F vec(0,0,-dist);
    Vector3F output = QVRotate(q, vec);

    return QPoint(_size.width()/2 + output.x, _size.height()/2 + output.z);
}

bool TFLOSMRenderer::ptInScreen(QPoint pt) const
{
    int border = 200;
    return pt.x() > -border && pt.x() < _size.width()+border
            && pt.y() >-border && pt.y() < _size.height()+border;
}

bool TFLOSMRenderer::ptInScreen(const GPSLocation& l) const
{
    return ptInScreen(toScreen(l));
}

void TFLOSMRenderer::perform(QPainter* p)
{
    for( auto& renderObject : _renderObjects)
    {
        if( p == nullptr)
            renderObject->clear();

        //if( getPixelLevel() >  renderObject->getPixelsPerMile())
        {
            if( p == nullptr)
                renderObject->updateCache();
            else
                renderObject->paint(*p);
        }
    }
}

void TFLOSMRenderer::calcBoundingBox(const GPSLocation &topLeft, const GPSLocation &bottomRight)
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
