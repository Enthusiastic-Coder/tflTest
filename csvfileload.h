#ifndef CSVFILELOAD_H
#define CSVFILELOAD_H

#include <string>
#include <QStringList>

class CSVFileLoad
{
public:
    virtual ~CSVFileLoad() = default;
    bool Load(const QString &sFilename, const int fieldCount, char separator=',');
    void interruptLoad();
    bool hasLoaded() const;
    void resetHasLoaded();
    QString getFilename() const;

    virtual void onLine(int lineNo, const QStringList& args) = 0;

private:
    bool _bInterruptLoad = false;
    volatile bool _bHasLoaded = false;
    QString _filename;
};

#endif // CSVFILELOAD_H
