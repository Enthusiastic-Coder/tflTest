#include "GPSBoundary.h"

GPSBoundary::GPSBoundary()
{

}

GPSBoundary::GPSBoundary(const GPSLocation &topLeft, const GPSLocation &bottomRight)
    :_topLeft(topLeft), _bottomRight(bottomRight)
{

}

bool GPSBoundary::contains(const GPSBoundary &boundary) const
{
    return false;
}
