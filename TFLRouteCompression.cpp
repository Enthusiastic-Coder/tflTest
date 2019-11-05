#include "TFLRouteCompression.h"

TFLRouteCompression::TFLRouteCompression(QObject *parent)
    : QObject(parent)
{

}

void TFLRouteCompression::produceCompressedOutput()
{

    emit finished();
}

