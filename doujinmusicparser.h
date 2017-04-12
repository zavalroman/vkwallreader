#ifndef DOUJINMUSICPARSER_H
#define DOUJINMUSICPARSER_H

#include <QObject>

class DoujinmusicParser : public QObject
{
    Q_OBJECT
public:
    explicit DoujinmusicParser(QObject *parent = 0);

    void postsParseStart(int rangeFrom, int rangeTo, int epoch);

signals:

public slots:

private:

};

#endif // DOUJINMUSICPARSE_H
