#include "NetworkRailScheduleJSON.h"

#include <QFile>
#include <QJsonDocument>


void NetworkRailScheduleJSON::load(const QString &filename)
{
    QFile file(filename);
    if( !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << Q_FUNC_INFO << "- load **failed**";
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if( doc.isNull())
    {
        qDebug() << Q_FUNC_INFO << "- fromJson **failed**";
        return;
    }


}
