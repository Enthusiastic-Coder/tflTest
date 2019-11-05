#ifndef TFLROUTECOMPRESSION_H
#define TFLROUTECOMPRESSION_H

#include <QObject>
#include <QJsonDocument>

class TFLRouteCompression : public QObject
{
    Q_OBJECT

public:
    explicit TFLRouteCompression(QObject* parent=nullptr);

    void produceCompressedOutput();

private:
    QString processLineObject(QJsonDocument document);

signals:
    void finished(QString output);
};

#endif // TFLROUTECOMPRESSION_H
