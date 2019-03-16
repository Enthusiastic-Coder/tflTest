#ifndef GPSBOUNDARY_H
#define GPSBOUNDARY_H

#include <GPSLocation.h>
#include <QPair>

class GPSBoundary
{
public:
    GPSBoundary();
    GPSBoundary(const GPSLocation& topLeft, const GPSLocation& bottomRight);
    GPSBoundary(const QPair<GPSLocation,GPSLocation>& box);

    bool contains(const GPSBoundary& boundary) const;

private:
    GPSLocation _topLeft;
    GPSLocation _bottomRight;
};

#endif // GPSBOUNDARY_H
