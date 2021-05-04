#include "csvfileload.h"
#include <algorithm>
#include <QFile>
#include <QDebug>

bool CSVFileLoad::Load(const QString& sFilename, const int fieldCount, char separator)
{
    _bHasLoaded = false;
    _filename = sFilename;

    QFile inFile(sFilename);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to load : " << sFilename;
        return false;
    }

    int iLineNo = 0;
    std::vector<std::string> dataLine;
    dataLine.resize(fieldCount);

    while (!inFile.atEnd())
    {
        if( _bInterruptLoad )
            return false;

        QString line = inFile.readLine();
        line = line.trimmed();

        if( line.isEmpty())
            continue;

        line.remove('\n');
        line.remove('"');

        onLine(iLineNo, line.split(separator));
        iLineNo++;
    }

    _bHasLoaded = true;

    return true;
}

void CSVFileLoad::interruptLoad()
{
    _bInterruptLoad = true;
}

bool CSVFileLoad::hasLoaded() const
{
    return _bHasLoaded;
}

void CSVFileLoad::resetHasLoaded()
{
    _bHasLoaded = false;
}

QString CSVFileLoad::getFilename() const
{
    return _filename;
}
