#include "GPSBoundary.h"

GPSBoundary::GPSBoundary()
{

}

GPSBoundary::GPSBoundary(const GPSLocation &topLeft, const GPSLocation &bottomRight)
    :_topLeft(topLeft), _bottomRight(bottomRight)
{

}

GPSBoundary::GPSBoundary(const QPair<GPSLocation, GPSLocation> &box)
   : GPSBoundary( box.first, box.second)
{

}

bool GPSBoundary::contains(const GPSBoundary &boundary) const
{
    int X = 0;
    if( boundary._topLeft._lng < this->_topLeft._lng)
        X--;
    else if( boundary._bottomRight._lng > this->_bottomRight._lng)
        X++;

    if( boundary._bottomRight._lng < this->_topLeft._lng)
        X--;
    else if( boundary._topLeft._lng > this->_bottomRight._lng)
        X++;

    int Y = 0;
    if( boundary._topLeft._lat > this->_topLeft._lat)
        Y--;
    else if( boundary._bottomRight._lat < this->_bottomRight._lat)
        Y++;

    if( boundary._bottomRight._lat > this->_topLeft._lat)
        Y--;
    else if( boundary._topLeft._lat < this->_bottomRight._lat)
        Y++;

    return !(X == -2 || Y == -2 || X ==2 || Y ==2);
}
