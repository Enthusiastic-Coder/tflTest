#include "TFLOSMRenderer.h"
#include "OSMData.h"
#include <QSettings>

#define RENDER_TYPE(name, obj, func) _renderObjects.push_back(obj = new name(func));

TFLOSMRenderer::TFLOSMRenderer(OSMData *osmData):
    _osmData(osmData)
{
}

void TFLOSMRenderer::init()
{
    const auto& d = *_osmData;

    RENDER_TYPE(OSMRenderAeroway, _aeroway, d.getAeroWay());
    RENDER_TYPE(OSMRenderAeroRunway, _aerorunway, d.getAeroRunway());
    RENDER_TYPE(OSMRenderWater, _water, d.getWater());
    RENDER_TYPE(OSMRenderFootway, _footway, d.getFootway());
    RENDER_TYPE(OSMRenderCycleWay, _cycleWay, d.getCycleWay());
    RENDER_TYPE(OSMRenderPedestrian, _pedestrian, d.getPedestrian());
    RENDER_TYPE(OSMRenderTertiary, _tertiary, d.getTertiary());
    RENDER_TYPE(OSMRenderResidential, _residential, d.getResidential());
    RENDER_TYPE(OSMRenderSecondaryRoad, _secondary, d.getSecondary());
    RENDER_TYPE(OSMRenderPrimaryRoad, _primary, d.getPrimary());
    RENDER_TYPE(OSMRenderMotorWay, _motorway, d.getMotorway());

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
