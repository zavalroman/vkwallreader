
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QEventLoop>
#include <QFile>

#include "../ibpp/tests/C++/qt-firebird.h"

#include "doujinmusicparser.h"
#include "mismatch.h"

DoujinmusicParser::DoujinmusicParser(QObject *parent, Interface* mainWindow) : QObject(parent)
{
    root = mainWindow;
}

void DoujinmusicParser::textPrepare(QString *text)
{
    for ( int i = 0; i < text->size(); i++) {
        if (text->at(i) == 39) {
            text->insert(i, 39); //экранируем апостроф
            i++;
        }
    }
}

void DoujinmusicParser::postsParseStart(int rangeFrom, int rangeTo, int epoch)
{
    Firebird fb;
    QStringList textList;
    QList<int> intList;
    int id;

    QString event, tag_event, tag_genre, circle, album, disc, part, other;
    Mismatch *mismatch;
    int audio_count = 0;
    QString event_save, circle_save, album_save, genre_save;

    QStringList albumBuffer;
    QList<int> albumPostId;
    QList<int> acount;

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
        event.clear();
        tag_event.clear();
        tag_genre.clear();
        circle.clear();
        album.clear();
        part.clear();

        bool openMismatch = false;
        QRegularExpression re;
        QRegularExpressionMatch match;
        switch (epoch) {
        case 0: {
            re.setPattern("\\[(.+)\\]");
            match = re.match(post_text);
            if (match.hasMatch()) {
                event = match.captured(1);
            } else {
                openMismatch = true;
            }
            break;
        }
        case 1: {
            re.setPattern("\\[(.+)\\]%%(.+)\\s+[-–—]\\s+(.+)\\s(\\([Pp]art\\s\\d+\\))?([.]*)?");
            match = re.match(post_text);
            if (match.hasMatch()) {
                event  = match.captured(1);
                circle = match.captured(2);
                album  = match.captured(3);
                part   = match.captured(4);
                other  = match.captured(5);
            } else {
                openMismatch = true;
            }
            break;
        }
        case 2: {
            re.setPattern("\\[(.+)\\]\\s+(#[^\\s]+@doujinmusic)\\s*%%(.+)\\s+[-–—]\\s+(.+)\\s(\\([Pp]art\\s\\d+\\))?([.]*)?");
            match = re.match(post_text);
            if (match.hasMatch()) {
                event     = match.captured(1);
                tag_event = match.captured(2);
                circle    = match.captured(3);
                album     = match.captured(4);
                part      = match.captured(5);
                other     = match.captured(6);
            } else {
                openMismatch = true;
            }
            break;
        }
        case 3: {
            //re.setPattern("^\\[(.+)\\]\\s+(#[^\\s]+@doujinmusic)\\s*%%(.+)%%(.+)\\s+[-–—]\\s+(.+)\\s?(\\([Pp]art\\s\\d+\\))?.*$");
            re.setPattern( "^\\[(.+)\\]\\s+(#[^\\s]+@doujinmusic)\\s*%%(.+)%%(.+)\\s+[-–—]\\s+(.++)\\s*(\\([Pp]art\\s\\d+\\))?\\s*%?%?(.+)?" );
            match = re.match(post_text);
            if (match.hasMatch()) {
                event     = match.captured(1);
                tag_event = match.captured(2);
                tag_genre = match.captured(3);
                circle    = match.captured(4);
                album     = match.captured(5);
                part      = match.captured(6);
            } else {
                openMismatch = true;
            }
            break;
        }
        }
        qDebug() << "match (event) :" << event;
        qDebug() << "match (#event):" << tag_event;
        qDebug() << "match (#genre):" << tag_genre;
        qDebug() << "match (circle):" << circle;
        qDebug() << "match (album) :" << album;
        qDebug() << "match (part)  :" << part;
        if (openMismatch) {
            qDebug() << "mismatch";
        }

        re.setPattern( "\\s+[-–—]\\s+" ); //проверка для случая, если альбом и круг не в том месте разделены
        match = re.match(album);
        if (circle.size() == 0 || album.size() == 0 || match.hasMatch()) {
            openMismatch = true;
        }
        if (openMismatch) {
            mismatch = new Mismatch(root, &fb, post_text, id);
            if (mismatch->exec()) {
                event = mismatch->correctEvent;
                circle = mismatch->correctCircle;
                album = mismatch->correctAlbum;
                disc = mismatch->correctDisc;
                part = mismatch->correctPart;
            } else {
                qDebug() << "Cancel?";
            }
            delete mismatch;
        }
        textPrepare(&event);
        textPrepare(&circle);
        textPrepare(&album);

        if (albumBuffer.size() > 0 && !album.contains(albumBuffer.back().left(3))) {
            if ( audio_count>0 ) {
                trackInsertPrepare(&fb, &albumPostId, audio_count, event_save, circle_save, album_save, genre_save);
            } else { qDebug() << "ALBUM HAS NO ANY TRACKS"; }
            audio_count = 0;
            albumBuffer.clear();
            albumPostId.clear();
        }

        albumBuffer.push_back(album);
        statement = "SELECT audio_count FROM vkpost WHERE id = " + QString::number(id);
        fb.query(statement, &acount);

        if (acount.size() > 0 && acount.at(0) > 0) {
            albumPostId.push_back(id);
            audio_count += acount.at(0);
        } else {
            qDebug() << "SOME TRUBLES";
        }
        if (event.size() > 0)
            event_save = event;
        if (circle.size() > 0)
            circle_save = circle;
        if (album.size() > 0)
            album_save = album;
        if (tag_genre.size() > 0)
            genre_save = tag_genre;
        //final

        id++;
    } // foreach
}

int DoujinmusicParser::getPostDuration(Firebird *fb, int id)
{
    int total = 0;
    QList<int> durations;
    QString statement = "SELECT duration FROM track WHERE vkpost_id = " + QString::number(id);
    fb->query(statement, &durations);
    for (int i = 0; i < durations.size(); i++) {
        total += durations.at(i);
    }
    return total;
}

void DoujinmusicParser::trackInsertPrepare(Firebird *fb, QList<int>* albumPostId, int audio_count, QString &event, QString &circle, QString &album, QString &genre)
{
    QString statement;
    QList<int> index;
    QStringList textList;
    int duration = 0;

    if (audio_count == 0 || albumPostId->size() == 0) {
        qDebug() << "0 TRACKS! return";
        return;
    }

    /*************************************GET DURATION**************************************/
    for (int i = 0; i < albumPostId->size(); i++)
        duration += getPostDuration(fb, albumPostId->at(i));

    /**************************************GET EVENT****************************************/
    int event_id;
    if (event.size()>2) {
        statement = "SELECT id FROM event WHERE name = '" + event + "'";
        fb->query(statement,&index);
        if (index.size()==0) {
            statement = "INSERT INTO event(name) VALUES ('" + event + "')";
            fb->query(statement);
            statement = "SELECT MAX(id) FROM event";
            fb->query(statement,&index);
        }
        event_id = index.at(0);
    } else {
        event_id = 1; // id1 = null
    }

    /***************************************GET CIRCLE******************************************/
    statement = "SELECT id FROM circle WHERE name_unic = '" + circle + "'";
    fb->query(statement,&index);
    if (index.size()==0) {
        statement = "SELECT id FROM circle WHERE name_roma = '" + circle + "'";
        fb->query(statement,&index);
        if (index.size()==0) {
            statement = "INSERT INTO circle(name_unic) VALUES('" + circle + "')";
            fb->query(statement);
            statement = "SELECT MAX(id) FROM circle";
            fb->query(statement,&index);
        }
    }
    int circle_id = index.at(0);

    /**************************************INSERT ALBUM****************************************/
    QList<int> unix_time; //int: 2 147 483 647 -- 19th January 2038 03:14:07 AM
    statement = "SELECT unix_time FROM vkpost WHERE id = " + QString::number(albumPostId->at(0));
    fb->query(statement, &unix_time);
    statement = "INSERT INTO ALBUM(title_unic,circle_id,track_count,duration,event_id,vktime) "
        "VALUES ('"+album+"',"+QString::number(circle_id)+","+QString::number(audio_count)+","+QString::number(duration)+","+QString::number(event_id)+","+QString::number(unix_time.at(0))+")";
    fb->query(statement);
    statement = "SELECT MAX(id) FROM album";
    fb->query(statement, &index);
    int album_id = index.at(0);
    album_id_global = album_id;

    /***********************************DOWNLOAD COVER****************************************/
    QEventLoop loop;
    statement = "SELECT photo_604 FROM vkphoto WHERE id = " + QString::number(albumPostId->at(0));
    if (fb->query(statement, &textList) && textList.size() > 0) {
        //qDebug() << "image url" << textList[0];
        QUrl url(textList[0]);
        QNetworkRequest request(url);
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        //qDebug() << "DOWNLOAD BEGIN";
        manager->get(request);
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(photoDownloadFinished(QNetworkReply*)));
        connect(this, SIGNAL(photoSaved()), &loop, SLOT(quit()));
    } else { qDebug() << "sql query error"; }

     /**************************************INSERT TRACK****************************************/
    for (int i = 0; i < albumPostId->size(); i++) {
        statement = "SELECT id FROM track WHERE vkpost_id = " + QString::number(albumPostId->at(i));
        fb->query(statement,&index);
        //qDebug() << "album post id =" << albumPostId->at(i);
        if (index.size()==0) {
            qDebug() << "POST HAS NO TRACKS? break";
            break;
        }
        for (int j = 0; j < index.size(); j++) {
            //trackInsert(fb, index.at(j), album_id, circle_id);
            //statement = "INSERT INTO track(vktrack_id,title_unic,album_id,circle_id,duration) VALUES("+QString::number(vktrack_id)+",'"+track_title+"',"+QString::number(album_id)+","+QString::number(circle_id)+","+QString::number(duration)+")";
            statement = "UPDATE track SET circle_id = "+QString::number(circle_id)+" WHERE id = "+QString::number(index.at(j));
            fb->query(statement);
            statement = "UPDATE track SET album_id = "+QString::number(circle_id)+ " WHERE id = "+QString::number(index.at(j));
            fb->query(statement);
        }
    }

    loop.exec(); //wait for end download cover
}

void DoujinmusicParser::photoDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        return;
    }
    QVariant attribute = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (attribute.isValid()) {
        QUrl url = attribute.toUrl();
        qDebug() << "must go to:" << url;
        return;
    }
    //qDebug() << "Content type:" << reply->header(QNetworkRequest::ContentTypeHeader).toString();
    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);
    QFile file("covers/" + QString::number(album_id_global));
    file.open(QIODevice::WriteOnly);

    pixmap.save(&file, "jpg");
    file.close();
    emit photoSaved();
}


void DoujinmusicParser::trackInsert(Firebird* fb, int vktrack_id, int album_id, int circle_id)
{
    QString statement;
    QStringList textList;
    QList<int> intList;
    int duration;

    statement = "SELECT duration FROM vktrack WHERE id = " + QString::number(vktrack_id);
    fb->query(statement, &intList);
    duration = intList.at(0);

    statement = "SELECT title FROM vktrack WHERE id = " + QString::number(vktrack_id);
    fb->query(statement,&textList);
    QString track_title = textList.at(0);

    textPrepare(&track_title);

    statement = "INSERT INTO track(vktrack_id,title_unic,album_id,circle_id,duration) VALUES("+QString::number(vktrack_id)+",'"+track_title+"',"+QString::number(album_id)+","+QString::number(circle_id)+","+QString::number(duration)+")";
    fb->query(statement);
}
