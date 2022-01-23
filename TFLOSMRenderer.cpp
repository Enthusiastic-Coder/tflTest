#include "TFLOSMRenderer.h"
#include "OSMData.h"
#include <QSettings>

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

void TFLOSMRenderer::unInit()
{
    for( auto& renderObject : _renderObjects)
        renderObject->unInit();
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
    return QPoint();
}

bool TFLOSMRenderer::ptInScreen(QPoint pt) const
{
    return false;
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
