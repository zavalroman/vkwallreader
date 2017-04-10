
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "../ibpp/tests/C++/qt-firebird.h"

#include "doujinmusicparse.h"

DoujinmusicParse::DoujinmusicParse(QObject *parent) : QObject(parent)
{

}

void DoujinmusicParse::postsParse(int rangeFrom, int rangeTo, int epoch)
{
    Firebird fb;
    QStringList textList;
    QList<int> intList;
    int id;

    QString event, circle, album, disc, part;

    QString statement = "SELECT post_text FROM vkpost WHERE id BETWEEN "+QString::number(rangeFrom)+" AND "+QString::number(rangeTo);
    fb.query(statement, &textList);

    id = rangeFrom;
    statement = "SELECT MIN(id) FROM vkpost";
    fb.query(statement, &intList);
    if (id < intList.at(0))
        id = intList.at(0);

    foreach(const QString& post_text, textList) {
        qDebug() << post_text;

/*

epoch 3
[Touhou Kouroumu 8] #TK8@doujinmusic
#metal #touhou
IRON ATTACK! - PAPER MOON

epoch 2
[Touhou Fuhai Komachi 2] #TFK2@doujinmusic
Sonic Hybrid Orchestra - 東方の嵐～TOHO TEMPEST (part 2)

epoch 1
[C85]
Kishida Kyoudan & The Akeboshi Rockets (Studio 2k) - 2007-2010 東方アレンジベスト (part 1)

epoch 0
[M3-26]

Bassy - ワンダー・フルワールド.rar1

*/
        QRegularExpression re;
        QRegularExpressionMatch match;
        switch (epoch) {
        case 0: {
            re.setPattern("\\[(.+)\\]");
            match = re.match(post_text);
            if (match.hasMatch()) {
                qDebug() << "match epoch 0:" << match.captured(1);
            } else {
                qDebug() << "match epoch 0: HAS NOT MATCHED";
            }
            break;
        }
        case 1: {
            re.setPattern("\\[(.+)\\]%%(.+)\\s+[-–—]\\s+(.+)\\s+?(\\([Pp]art\\s\\d\\))?%?%?(.*)?");
            match = re.match(post_text);
            if (match.hasMatch()) {
                qDebug() << "match epoch 1 (event) :" << match.captured(1);
                qDebug() << "match epoch 1 (circle):" << match.captured(2);
                qDebug() << "match epoch 1 (album) :" << match.captured(3);
                qDebug() << "match epoch 1 (part)  :" << match.captured(4);
                qDebug() << "match epoch 1 (other) :" << match.captured(5);
            } else {
                qDebug() << "match epoch 1: HAS NOT MATCHED";
            }
            break;
        }
        case 2: {
            re.setPattern("\\[(.+)\\]\\s+(#[^\\s]+@doujinmusic)\\s*%%(.+)\\s+[-–—]\\s+(.+)\\s+?(\\([Pp]art\\s\\d\\))?%?%?(.*)?");
            match = re.match(post_text);
            if (match.hasMatch()) {
                qDebug() << "match epoch 2 (event) :" << match.captured(1);
                qDebug() << "match epoch 2 (#event):" << match.captured(2);
                qDebug() << "match epoch 2 (circle):" << match.captured(3);
                qDebug() << "match epoch 2 (album) :" << match.captured(4);
                qDebug() << "match epoch 2 (part)  :" << match.captured(5);
                qDebug() << "match epoch 2 (other) :" << match.captured(6);
            } else {
                qDebug() << "match epoch 2: HAS NOT MATCHED";
            }
            break;
        }
        case 3: {
            re.setPattern("\\[(.+)\\]\\s+(#[^\\s]+@doujinmusic)\\s*%%(.*)%%(.+)\\s+[-–—]\\s+(.+)\\s+?(\\([Pp]art\\s\\d\\))?%?%?(.*)?");
            match = re.match(post_text);
            if (match.hasMatch()) {
                qDebug() << "match epoch 3 (event) :" << match.captured(1);
                qDebug() << "match epoch 3 (#event):" << match.captured(2);
                qDebug() << "match epoch 2 (#genre):" << match.captured(3);
                qDebug() << "match epoch 3 (circle):" << match.captured(4);
                qDebug() << "match epoch 3 (album) :" << match.captured(5);
                qDebug() << "match epoch 3 (part)  :" << match.captured(6);
                qDebug() << "match epoch 3 (other) :" << match.captured(7);
            } else {
                qDebug() << "match epoch 3: HAS NOT MATCHED";
            }
            break;
        }
        }
    } // foreach
}
