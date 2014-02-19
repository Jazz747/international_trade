#include <QCoreApplication>

#include <QFile>
#include <QByteArray>
#include <QStringList>
#include <QDebug>

#include "currency.h"
#include "xmlratereader.h"

#include <math.h>

float roundToEven(float value) {
    // Convert to cents
    value *= 100.0f;
    // Only need tie-breaker if the fraction is
    // 0.5 exactly
    if (qFuzzyCompare(fabsf(value - truncf(value)), 0.5f)) {
        // return nearest even
        float a = ceilf(value);
        if (int(a) % 2 == 0)
            return a / 100.0f;
        return floorf(value) / 100.0f;
    }

    return roundf(value) / 100.0f;
}

void testRounding() {
    QList<float> nums;
    nums << 24.50f << 23.67f << 23.50f << 23.35f << 23.00f << 0 << -23.00f << -23.35f << -23.50f << -23.67f << -24.50f;
    foreach (float f, nums)
        qDebug() << f << roundToEven(f);
}

void total(QString filename, QString item, QString currency) {
    float sum = 0.0f;
    QFile csv(filename);
    csv.open(QIODevice::ReadOnly);
    while (true) {
        QString line = csv.readLine();
        if (line.isEmpty())
            break;
        QStringList split = line.split(QRegExp("[, \n]"));
        if (split.size() < 4 || split[1] != item)
            continue;
        bool ok = false;
        float amount = split[2].toFloat(&ok);
        Currency *c = Currency::get(split[3]);
        if (!ok) {
            qDebug() << "Failed to parse amount:" << split[2];
            continue;
        }
        if (!c || !c->hasRate(currency)) {
            qDebug() << "No known conversion from" << split[3] << "to" << currency;
            continue;
        }
        qDebug() << sum << amount << c->to(currency) << "+="
                 << amount * c->to(currency) << split[0];
        sum += roundToEven(amount * c->to(currency));
    }
    qDebug() << "sum:" << sum;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    testRounding();

    //XmlRateReader reader("/Users/hchapman/Desktop/international-trade/doc/test_rates.xml");

    XmlRateReader reader("/Users/hchapman/Desktop/international-trade/doc/SAMPLE_RATES.xml");
    //XmlRateReader reader("/Users/hchapman/Desktop/international-trade/doc/RATES.xml");
    reader.read();

    Currency::printMap();
    Currency::fillInTable();
    Currency::printMap();

    // Read in sales csv
    // Find total sales in USD for DM1182
    total("/Users/hchapman/Desktop/international-trade/doc/SAMPLE_TRANS.csv", "DM1182", "USD");
    // for each line, split, if matches item, get currency, convert, add to total

    return 0;
//    return a.exec();
}