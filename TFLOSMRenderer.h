#ifndef TFLOSMRENDERER_H
#define TFLOSMRENDERER_H

#include "OSMRendererBase.h"
#include <vector>

class TFLOSMRenderer
{
public:
    TFLOSMRenderer(OSMData* osmData);

    void init();
    void updateCache();
    void paint(QPainter& p);
    void paintText(QPainter& p);

    void setPixelLevel(float p);
    float getPixelLevel() const;

    void setMapNight(bool n);
    bool isMapNight() const;

    float getCompassValue() const;

    void setLocation(const GPSLocation& l);
    QPoint toScreen(const GPSLocation &l) const;
    bool ptInScreen(QPoint pt) const;
    bool ptInScreen(const GPSLocation &l) const;

protected:
    void perform(QPainter *p);

private:
    OSMData* _osmData;
    float _pixelLevel = 1.0f;
    GPSLocation _location;
    bool _isNight = false;
    std::vector<OSMRendererBase*> _renderObjects;
    OSMRenderMotorWay* _motorway = nullptr;
    OSMRenderSecondary* _secondary = nullptr;
    OSMRenderTertiary* _tertiary = nullptr;
    OSMRenderPrimary* _primary = nullptr;
    OSMRenderResidential* _residential = nullptr;
    OSMRenderFootway* _footway = nullptr;
    OSMRenderWater* _water = nullptr;
    OSMRenderAeroWay* _aeroway = nullptr;
    OSMRenderAeroRunway* _aerorunway = nullptr;
    OSMRenderCycleWay* _cycleWay = nullptr;
    OSMRenderPedestrian* _pedestrian = nullptr;
};

#endif // TFLOSMRENDERER_H
