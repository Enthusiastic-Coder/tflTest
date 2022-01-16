#ifndef OSMDATA_H
#define OSMDATA_H

#include <vector>
#include <QString>
#include <GPSLocation.h>
#include <memory>

struct OSM_WAYPOINT
{
    std::vector<std::pair<int,QString>> tags;
    std::vector<GPSLocation> gpsPts;
    std::vector<float> distances;
    std::vector<int> bearings;
};

using WAYPOINTS = std::vector<std::unique_ptr<OSM_WAYPOINT> >;

#define OSMType(name, wayPts) \
    void import##name(const QString& filename) { import(filename, wayPts);}\
    const WAYPOINTS& get##name() const { return wayPts;}

class OSMData
{
public:
    OSMData();

    OSMType(Motorway, _osmMotorway)
    OSMType(Primary, _osmPrimary)
    OSMType(Secondary, _osmSecondary)
    OSMType(Tertiary, _osmTertiary)
    OSMType(Residential, _osmResidential)
    OSMType(Footway, _osmFootway)
    OSMType(Water, _osmWater)
    OSMType(AeroWay, _osmAeroway)
    OSMType(AeroRunway, _osmAeroRunway)
    OSMType(CycleWay, _osmCycleway)
    OSMType(Pedestrian, _osmPedestrian);

protected:
    void import(const QString &filename, WAYPOINTS &wayPoints, bool bAllowPoints=false);

private:
    WAYPOINTS _osmMotorway;
    WAYPOINTS _osmPrimary;
    WAYPOINTS _osmSecondary;
    WAYPOINTS _osmTertiary;
    WAYPOINTS _osmResidential;
    WAYPOINTS _osmFootway;
    WAYPOINTS _osmWater;
    WAYPOINTS _osmAeroway;
    WAYPOINTS _osmAeroRunway;
    WAYPOINTS _osmCycleway;
    WAYPOINTS _osmPedestrian;
    QuarternionF _topLeft;
    QuarternionF _bottomRight;
};

#endif // OSMDATA_H
