#ifndef DOUJINMUSICPARSER_H
#define DOUJINMUSICPARSER_H

#include <QObject>
#include "interface.h"

class Firebird;

class DoujinmusicParser : public QObject
{
    Q_OBJECT
public:
    explicit DoujinmusicParser(QObject *parent = 0, Interface* mainWindow = nullptr);

    void postsParseStart(int rangeFrom, int rangeTo, int epoch);

signals:
    void photoSaved();

public slots:
    void photoDownloadFinished(QNetworkReply*);

private:
    int album_id_global; //for photo name

    void textPrepare(QString*);
    Interface* root;
    int getPostDuration(Firebird* fb, int id);
    void trackInsertPrepare(Firebird* fb, QList<int>* albumPostId, int audio_count, QString &event, QString &circle, QString &album);
    void trackInsert(Firebird* fb, int vktrack_id, int album_id, int circle_id);
};

#endif // DOUJINMUSICPARSE_H
