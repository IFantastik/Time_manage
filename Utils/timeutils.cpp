#include "timeutils.h"

QString TimeUtils::pluralRu(int typeTime, const QString& one, const QString& few, const QString& many)
{
    int mod100 = typeTime % 100;
    if (mod100 >= 11 && mod100 <=14) return many;

    int mod10 = typeTime % 10;
    if (mod10 == 1) return one;
    if (mod10 >= 2 && mod10 <=4) return few;

    return many;
}

QString TimeUtils::formatWorkedRu(int totalMinutes)
{
    if(totalMinutes <= 0) return QString();
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;

    QString res;
    if (hours > 0) {
        res += QString("%1 %2").arg(hours).arg(pluralRu(hours, "час", "часа", "часов"));
    }
    if (minutes > 0) {
        if (!res.isEmpty()) res += " ";
        res += QString("%1 %2").arg(minutes).arg(pluralRu(minutes, "минута", "минуты", "минут"));
    }

    return res;
}
