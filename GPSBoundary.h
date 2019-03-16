#ifndef GPSBOUNDARY_H
#define GPSBOUNDARY_H

#include <GPSLocation.h>

class GPSBoundary
{
public:
    GPSBoundary();
    GPSBoundary(const GPSLocation& topLeft, const GPSLocation& bottomRight);

    bool contains(const GPSBoundary& boundary) const;

private:
    GPSLocation _topLeft;
    GPSLocation _bottomRight;
};

#endif // GPSBOUNDARY_H
