#ifndef VKAPI_H
#define VKAPI_H

#include <QObject>
#include <QNetworkReply>

#include "vkpost.h"
#include "../qt-json/json.h"

using QtJson::JsonObject;
using QtJson::JsonArray;

class VkApi : public QObject
{
    Q_OBJECT
public:
    explicit VkApi(QObject *parent = 0);

    void setSourceId(QString& owner_id, QString& domain_) {
        ownerId = owner_id;
        domain = domain_;
    }
    void setToken(QString& token_) {
        token = token_;
    }
    void setTimeRange(uint start, uint end) {
        startTime = start;
        endTime = end;
    }
    void setAdminCommentAudio() {
        adminCommentAudio = true;
    }
    void setCommentText() {
        needCommentText = true;
    }

    void delay(int msec) const;
    void wallGet(QString& cycles, QString& strOffset, QString& count);
    void getDoc(QString& docs);

signals:
    void vkPostReceived(Vkpost*);
    void message(QString);
    void docUrl(QUrl);
    void cyclePercent(int);
    void currentCycle(int, int);

public slots:
    void stopWall();

private:
    struct Audio { // for comment's attachment audios
        QString id, owner_id, artist, title;
        int duration;
    } audio;

    struct Comment {
        QString commentator;
        QString text;
        int likes;
    } comment;

    bool scanStop, replyParsed;
    bool commentAudioComplete;

    QString ownerId, domain, token;
    uint startTime, endTime;
    bool adminCommentAudio, needCommentText;
    JsonObject jsonResponse;
    JsonArray jsonTracks; // ?
    int gottenCount;

    QList<Audio> audios;
    QList<QString> likes;
    QList<QString> shares;
    QList<Comment> comments;

    void jsonToVkpost(const JsonObject &result);
    void jsonToComment(const JsonObject &result);
    void jsonToLikes(const JsonObject &result);
    void jsonToShared(const JsonObject &result);
    void jsonToDoc(const JsonObject &result);
    void getComments(QString& postId, QString count);
    void getLikes(QString& itemId);
    void getShares(QString& itemId);

    bool checkDublicate(const JsonObject &result);

    void execute(QString parameters);
};

#endif // VKAPI_H
