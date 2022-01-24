#include "TFLOSMRenderer.h"
#include "OSMData.h"
#include <QSettings>
#include <MathSupport.h>

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
        renderObject->init();
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
    _pixelLevel = p;
}

float TFLOSMRenderer::getPixelLevel() const
{
    return _pixelLevel;
}

void TFLOSMRenderer::setMapNight(bool n)
{
    _isNight = n;
}

bool TFLOSMRenderer::isMapNight() const
{
    return _isNight;
}

void TFLOSMRenderer::setSize(QSize sz)
{
    _size = sz;
}

float TFLOSMRenderer::getCompassValue() const
{
    return 0.0f;
}

void TFLOSMRenderer::setLocation(const GPSLocation &l)
{
    _location = l;
}

QPoint TFLOSMRenderer::toScreen(const GPSLocation& l) const
{
    float brg = l.bearingFrom(_location);
    float dist = l.distanceTo(_location)/1000.0/1.609334 * _pixelLevel;

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

        if( getPixelLevel() >  renderObject->getPixelsPerMile())
        {
            if( p == nullptr)
                renderObject->updateCache();
            else
                renderObject->paint(*p);
        }
    }
}
