#ifndef TIMEUTILS_H
#define TIMEUTILS_H
#include <QString>

class TimeUtils
{
public:
    static QString formatWorkedRu(int totalMinutes);
    static QString pluralRu(int typeTime, const QString& one, const QString& few, const QString& many);

};

#endif // TIMEUTILS_H
