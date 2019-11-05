#ifndef TFLROUTECOMPRESSION_H
#define TFLROUTECOMPRESSION_H

#include <QObject>

class TFLRouteCompression : public QObject
{
    Q_OBJECT

public:
    explicit TFLRouteCompression(QObject* parent=nullptr);

    void produceCompressedOutput();

signals:
    void finished();
};

#endif // TFLROUTECOMPRESSION_H
