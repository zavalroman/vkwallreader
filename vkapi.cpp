#include "vkapi.h"

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>

#include "firecontrol.h"

VkApi::VkApi(QObject *parent) : QObject(parent)
{
    ownerId = "";
    domain = "";
    startTime = 0;
    endTime = 0;
    adminCommentAudio = false;
    needCommentText = false;
}

void VkApi::delay(int msec) const
{
    //qDebug() << "delay:" << msec;
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}

void VkApi::execute(QString parameters)
{
    QUrl vkRequest("https://api.vk.com/method/" + parameters);
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->get(QNetworkRequest(vkRequest));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    bool ok = false;
    jsonResponse = QtJson::parse(QString(reply->readAll()),ok).toMap();
    if (!ok) {
        emit message("E: reply not parsed ("+parameters+")");
        return;
    }
    QList<QString> resultkeys = jsonResponse.keys();
    if (resultkeys[0] == "error") {
        emit message("E: error received from vk ("+parameters+")");
        return;
    }
    /* does not work with docs.getById
    JsonObject jsonObject = jsonResponse["response"].toMap();
    JsonArray items = jsonObject["items"].toList();
    if (items.size() == 0) {
        emit message("E: reply has no items ("+parameters+")");
        //return;
    }
    */
    reply->deleteLater();
    delete manager;
    replyParsed = true;
}

void VkApi::wallGet(QString& cycles, QString& strOffset, QString& count)
{
    int offset = strOffset.toInt();
    scanStop = false;
    for (int i = 0; i < cycles.toInt(); i++) {
        emit currentCycle(i+1, cycles.toInt());
        do {
            delay(500);
            execute("wall.get?owner_id="+ownerId+"&domain="+domain+"&offset="+QString::number(offset)+"&count=1&filter=all&extended=0&v=5.52&access_token="+token);
            offset++;
        } while (checkDublicate(jsonResponse));
        offset--;
        delay(500);
        replyParsed = false;
        execute("wall.get?owner_id="+ownerId+"&domain="+domain+"&offset="+QString::number(offset)+"&count="+count+"&filter=all&extended=0&v=5.52&access_token="+token);
        if (replyParsed) {
            jsonToVkpost(jsonResponse);
        }
        delay(300);
        offset += count.toInt();
        if (scanStop) {
            emit message("Wallscan: cycle stopped. Current cycle: " + QString::number(i));
            break;
        }
    }
    qDebug() << "wallGet finish";
}

void VkApi::getComments(QString& postId, QString count)
{
    if (ownerId[0] != "-"){
        emit message("E: getComments: ownerId not exist (postId: "+postId+")");
        return;
    }
    replyParsed = false;
    execute("wall.getComments?owner_id="+ownerId+"&post_id="+postId+"&need_likes=1&offset=0&count="+count+"&v=5.62&access_token="+token);
    if (replyParsed) {
        jsonToComment(jsonResponse);
    }
}

void VkApi::getLikes(QString& itemId)
{
    replyParsed = false;
    execute("likes.getList?type=post&owner_id="+ownerId+"&item_id="+itemId+
            "&filter=likes&friends_only=0&extended=0&offset=0&count=1000&skip_own=0&v=5.62&access_token="+token);
    if (replyParsed) {
        jsonToLikes(jsonResponse);
    }
}

void VkApi::getShares(QString& itemId)
{
    replyParsed = false;
    execute("likes.getList?type=post&owner_id="+ownerId+"&item_id="+itemId+
            "&filter=copies&friends_only=0&extended=0&offset=0&count=1000&skip_own=0&v=5.62&access_token="+token);
    if (replyParsed) {
        jsonToShared(jsonResponse);
    }
}

void VkApi::getDoc(QString &docs)
{
    qDebug() << "getDoc" << docs;
    replyParsed = false;
    execute("docs.getById?docs="+docs+"&v=5.62&access_token="+token);
    if (replyParsed) {
        jsonToDoc(jsonResponse);
    }
}

void VkApi::jsonToVkpost(const JsonObject &result)
{
    audios.clear();
    likes.clear();
    shares.clear();
    comments.clear();

    Vkpost* vkpost;
    /*
        qDebug() << "head begin";
        QList<QString> keys = head.keys();
        for (int i = 0; i < keys.size(); i++ )
           qDebug() << keys[i];
        qDebug() << "items end";
    */
    JsonObject jsonObject = result["response"].toMap();
    JsonArray items = jsonObject["items"].toList();

    for (int i = 0; i < items.size(); i++) {
        emit cyclePercent(int(((float)i/(float)items.size())*100));

        JsonObject head = items[i].toMap();
        /*
        while (head["copy_history"].toList().size() > 0) { // skip reposts
            i++;
            emit message("Skip repost");
            if (i>=items.size()) {
                emit message("W: While copy history: out of range");
                return;
            }
            head = items[i].toMap();
        }
        */
        if (endTime != 0)
            while (endTime < head["date"].toUInt()) { // skip out of range posts
                i++;
                if (i>=items.size()) {
                    emit message("W: While skip by time: out of range");
                    return;
                }
                head = items[i].toMap();
            }
        if (startTime != 0)
            if (startTime > head["date"].toUInt()) {
                emit message("W: While time range: so early. Stop please");
                return;
            }

        vkpost = new Vkpost(); // will be deleted in firecontrol

        vkpost->id = head["id"].toString();
        vkpost->from_id = head["from_id"].toString();
        vkpost->owner_id = head["owner_id"].toString();
        ownerId = vkpost->owner_id;                             // NB
        //vkpost->post_source = head["post_source"].toString();
        vkpost->date = head["date"].toUInt();
        vkpost->post_type = head["post_type"].toString();
        if (head["copy_history"].toList().size() > 0) {
            vkpost->text = "(REPOST) "+head["text"].toString();
        } else {
            vkpost->text = head["text"].toString();
        }
        vkpost->commentCount = head["comments"].toMap()["count"].toInt();
        vkpost->likes = head["likes"].toMap()["count"].toInt();
        vkpost->reposts = head["reposts"].toMap()["count"].toInt();

        JsonArray attachments = head["attachments"].toList();

        vkpost->audio_post = false;
        vkpost->photo_post = false;
        vkpost->poll_post = false;

        foreach (QVariant attachment, attachments) {
/**********************************PHOTO***********************************/
            if (attachment.toMap()["type"].toString() == "photo") {
                //qDebug() << "type: photo";
                JsonObject jsonPhoto = attachment.toMap()["photo"].toMap();
/*
            QList<QString> keys = jsonPhoto.keys();
            for (int i = 0; i < keys.size(); i++ )
                qDebug() << keys[i];
*/
                vkpost->addNewPhoto();
                vkpost->photos.back().id = jsonPhoto["id"].toString();
                vkpost->photos.back().owner_id = jsonPhoto["owner_id"].toString();

                vkpost->photos.back().photo_75 = jsonPhoto["photo_75"].toString();
                vkpost->photos.back().photo_130 = jsonPhoto["photo_130"].toString();
                vkpost->photos.back().photo_604 = jsonPhoto["photo_604"].toString();
                vkpost->photos.back().photo_807 = jsonPhoto["photo_807"].toString();
                vkpost->photos.back().photo_1280 = jsonPhoto["photo_1280"].toString();

                vkpost->photos.back().width = jsonPhoto["width"].toInt();
                vkpost->photos.back().height = jsonPhoto["height"].toInt();
                vkpost->photos.back().text = jsonPhoto["text"].toString();

                vkpost->photos.back().post_id = jsonPhoto["post_id"].toString();
                vkpost->photos.back().access_key = jsonPhoto["access_key"].toString();
                //user_id, album_id, date

                vkpost->photo_post = true;
            }
/**********************************POLL***********************************/
/*
            if (attachment.toMap()["type"].toString() == "poll") {
                qDebug() << "type: poll";
                JsonObject jsonPoll = attachment.toMap()["poll"].toMap();
                CREATE TABLE MELOMANYPOLL...
                vkpost->poll_post = true;
            }
*/
/**********************************AUDIO***********************************/
            if (attachment.toMap()["type"].toString() == "audio") {
                //qDebug() << "type: audio";
                JsonObject jsonAudio = attachment.toMap()["audio"].toMap();
                vkpost->addNewTrack();
                vkpost->tracks.back().id = jsonAudio["id"].toString();
                vkpost->tracks.back().owner_id = jsonAudio["owner_id"].toString();
                vkpost->tracks.back().artist = jsonAudio["artist"].toString();
                vkpost->tracks.back().title = jsonAudio["title"].toString();
                vkpost->tracks.back().duration = jsonAudio["duration"].toInt();
                //date...
                //vkpost->tracks.back().url = jsonAudio["url"].toString();
                vkpost->audio_post = true;

            }
/**********************************DOC***********************************/
            if (attachment.toMap()["type"].toString() == "doc") {
                //qDebug() << "type: doc";
                JsonObject jsonDoc = attachment.toMap()["doc"].toMap();
                vkpost->addNewDoc();
                vkpost->docs.back().id = jsonDoc["id"].toString();
                vkpost->docs.back().owner_id = jsonDoc["owner_id"].toString();
                vkpost->docs.back().title = jsonDoc["title"].toString();
                vkpost->docs.back().size = jsonDoc["size"].toInt();
                vkpost->docs.back().ext = jsonDoc["ext"].toString();
                vkpost->docs.back().url = jsonDoc["url"].toString();
                vkpost->docs.back().access_key = jsonDoc["access_key"].toString();
            }
            delay(500); // ?

        }
/********************************COMMENTS***********************************/
        if (vkpost->commentCount > 0) {
            commentAudioComplete = false;
            getComments(vkpost->id, QString::number(vkpost->commentCount));
            if (adminCommentAudio) {
                if (commentAudioComplete) {
                    for (int i = 0; i < audios.size(); ++i) {
                        vkpost->addNewTrack();
                        vkpost->tracks.back().id = audios[i].id;
                        vkpost->tracks.back().owner_id = audios[i].owner_id;
                        vkpost->tracks.back().artist = audios[i].artist;
                        vkpost->tracks.back().title = audios[i].title;
                        vkpost->tracks.back().duration = audios[i].duration;
                    }
                }
            }
        }
        for ( int j = 0; j < comments.size(); ++j ) {
            vkpost->addNewComment();
            vkpost->comments.back().commentator = comments[j].commentator;
            vkpost->comments.back().text = comments[j].text;
            vkpost->comments.back().likes = comments[j].likes;
        }
        comments.clear();
/*********************************LIKES*************************************/
        if (vkpost->likes > 0) {
            getLikes(vkpost->id);
            vkpost->whoLikes = likes;
        }
        if (vkpost->reposts > 0) {
            getShares(vkpost->id);
            vkpost->whoShared = shares;
        }
        //emit message(vkpost->text);
        emit vkPostReceived(vkpost);
        if (scanStop) {
            emit message("Post insert stopped. Counter value: " + QString::number(i));
            break;
        }
    }
}

void VkApi::jsonToComment(const JsonObject &result)
{
    JsonObject jsonObject = result["response"].toMap();
    JsonArray items = jsonObject["items"].toList();

    //bool nonAlbumComment = false;
    for (int i = 0; i < items.size(); i++) { // items size - количество комментариев в ответе, но это не точно
        JsonObject head = items[i].toMap();
        JsonObject likes = head["likes"].toMap();
        comment.likes = likes["count"].toInt();
        comment.commentator = head["from_id"].toString();
        if (needCommentText) {
            comment.text = head["text"].toString();
        } else {
            comment.text = "";
        }
        comments.push_back(comment);

        if (head["from_id"].toString() == ownerId) {
            JsonArray attachments = head["attachments"].toList();
            foreach (QVariant attachment, attachments) {
                if (attachment.toMap()["type"].toString() == "audio") {
                    JsonObject jsonAudio = attachment.toMap()["audio"].toMap();
                    audios.push_back(audio);
                    audios.back().id = jsonAudio["id"].toString();
                    audios.back().owner_id = jsonAudio["owner_id"].toString();
                    audios.back().artist = jsonAudio["artist"].toString();
                    audios.back().title = jsonAudio["title"].toString();
                    audios.back().duration = jsonAudio["duration"].toInt();
                    //date...
                    //audios.back().url = jsonAudio["url"].toString();
                }
            }
        }
    }
    /*
    if (audios.size() > 0)
        commentAudioComplete = true;
    */
}

void VkApi::jsonToLikes(const JsonObject &result)
{
    JsonObject jsonObject = result["response"].toMap();
    JsonArray items = jsonObject["items"].toList();

    for (int i = 0; i < items.size(); i++) {
        likes.push_back(items[i].toString());
    }
}

void VkApi::jsonToShared(const JsonObject &result)
{
    JsonObject jsonObject = result["response"].toMap();
    JsonArray items = jsonObject["items"].toList();

    for (int i = 0; i < items.size(); i++) {
        shares.push_back(items[i].toString());
    }
}

void VkApi::jsonToDoc(const QtJson::JsonObject &result)
{
    JsonArray jsonArray = result["response"].toList();
    JsonObject doc = jsonArray[0].toMap();

    QUrl url = doc["url"].toUrl();
    emit docUrl(url);
}

bool VkApi::checkDublicate(const QtJson::JsonObject &result)
{
    Firecontrol fc;

    JsonObject jsonObject = result["response"].toMap();
    JsonArray items = jsonObject["items"].toList();
    JsonObject head = items[0].toMap();

    QString receivedId = head["id"].toString();
    qDebug() << "receivedId" << receivedId;
    return fc.postExist(receivedId);
}

void VkApi::stopWall()
{
    scanStop = true;
}
