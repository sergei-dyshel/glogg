#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QTextStream>
#include "re2/re2.h"

int main(int , char *argv[])
{
    QRegularExpression pattern(argv[1]);
    // RE2 pattern(argv[1]);
    QFile inputFile(argv[2]);
    QString line;
    char buf[1024];
    qint64 lineLen;
    int count = 0;
    if (!inputFile.open(QIODevice::ReadOnly))
        return -1;
    while (!inputFile.atEnd()) {
        lineLen = inputFile.readLine(buf, sizeof(buf));
        line = QString::fromUtf8(buf, lineLen);
        if (pattern.match(line).hasMatch())
            count++;
        // if (pattern.Match(buf, 0, lineLen, RE2::UNANCHORED, nullptr, 0))
        //     count++;
    }
    qInfo() << count;
    inputFile.close();
    return 0;
}