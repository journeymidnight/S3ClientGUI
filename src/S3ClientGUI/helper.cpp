#include "helper.h"
#include <QStringList>

QString helper::formattedDataSize(qlonglong s, int precision) {
        static QStringList units = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
        int power=0;
        int base=1000;
        if (s) {
                //math.log(s, 1000)
                power = int(std::log10(qAbs(s)) / 3);
        }

        const QString number = power
        ? QString::number(s/ std::pow(double(base), power), 'f', qMin(precision, 3 * power))
        : QString::number(s);

        return number + QLatin1Char(' ') + units[power];
}

