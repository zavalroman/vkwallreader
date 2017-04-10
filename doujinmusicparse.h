#ifndef DOUJINMUSICPARSE_H
#define DOUJINMUSICPARSE_H

#include <QObject>

class DoujinmusicParse : public QObject
{
    Q_OBJECT
public:
    explicit DoujinmusicParse(QObject *parent = 0);

    void postsParse(int rangeFrom, int rangeTo, int epoch);

signals:

public slots:

private:

};

#endif // DOUJINMUSICPARSE_H
