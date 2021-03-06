#include "firecontrol.h"

#include <QDebug>

//класс переименовать

Firecontrol::Firecontrol(QObject *parent) : QObject(parent)
{

}

void Firecontrol::textPrepare(QString *text) const
{
    for ( int i = 0; i < text->size(); i++) {
        if (text->at(i) == 39) {
            text->insert(i, 39); //экранируем апостроф
            i++;
        }
        if (text->at(i) == 10)
            text->replace(i,1,"%%"); //заменяем перенос строки двумя процентами
    }
}

void Firecontrol::vkpostToDb(Vkpost* vkpost)
{
    Firebird fb;
    QList<int> index;

    QString statement = "SELECT id FROM vkgroup WHERE vk_id = '"+vkpost->from_id+"'";
    fb.query(statement, &index);

    if (index.size() == 0) {
        qDebug() << "ERROR: IN DB GROUP NOT EXIST";
        return;
    }
    textPrepare(&vkpost->text);
    statement = "INSERT INTO vkpost(vk_id,vkgroup_id,unix_time,post_text,likes,reposts,comments,photo_count,audio_count) "
                        "VALUES ('"+vkpost->id+"',"+QString::number(index.at(0))+","+QString::number(vkpost->date)+",'"+vkpost->text+"',"+QString::number(vkpost->likes)+","+QString::number(vkpost->reposts)+
                        ","+QString::number(vkpost->comments.size())+","+QString::number(vkpost->photos.size())+","+QString::number(vkpost->tracks.size())+")";
    fb.query(statement);

    index.clear();
    statement = "SELECT MAX(id) FROM vkpost";
    fb.query(statement, &index);
    if (vkpost->photos.size() > 0) {
        statement = "INSERT INTO vkphoto(vk_id,photo_75,photo_130,photo_604,photo_807,photo_1280,photo_2560,vkpost_id) VALUES('"
                + vkpost->photos[0].id + "','" + vkpost->photos[0].photo_75 + "','" + vkpost->photos[0].photo_130 + "','" + vkpost->photos[0].photo_604 +
                    "','" + vkpost->photos[0].photo_807 + "','" + vkpost->photos[0].photo_1280 + "','" + vkpost->photos[0].photo_2560 + "'," + QString::number(index.at(0)) + ")";
        fb.query(statement);
    } else {
        //statement = "INSERT INTO vkphoto(vk_id,vkpost_id) VALUES('null',"+QString::number(index.at(0))+")";
        //fb.query(statement);
    }

    for (int j = 0; j < vkpost->tracks.size(); j++) {
        textPrepare(&vkpost->tracks[j].artist);
        textPrepare(&vkpost->tracks[j].title);
        statement = "INSERT INTO track(vk_id,to_id,artist,title,vkpost_id) VALUES('"+vkpost->tracks[j].id+"','"+vkpost->tracks[j].owner_id+"','"+vkpost->tracks[j].artist +
                                "','"+vkpost->tracks[j].title+"',"+QString::number(index.at(0))+")";
        fb.query(statement);
    }
    for (int j = 0; j < vkpost->comments.size(); ++j) {
        statement = "INSERT INTO comments(vkpost_id,commentator,text,likes) VALUES("+QString::number(index.at(0))+",'"+vkpost->comments[j].commentator+"','"+vkpost->comments[j].text+"',"+QString::number(vkpost->comments[j].likes)+")";
        fb.query(statement);
    }
    for (int j = 0; j < vkpost->docs.size(); ++j) {
        textPrepare(&vkpost->docs[j].title);
        statement = "INSERT INTO vkdoc(vkpost_id,vk_id,owner_id,title,size,ext,access_key) VALUES("+QString::number(index.at(0))+
                ",'"+vkpost->docs.at(j).id+"','"+vkpost->docs.at(j).owner_id+"','"+vkpost->docs.at(j).title+"',"+QString::number(vkpost->docs.size())+
                ",'"+vkpost->docs.at(j).ext+"','"+vkpost->docs.at(j).access_key+"')";
        fb.query(statement);
    }
    delete vkpost;
}

void Firecontrol::getDocId(QString rangeBegin, QString rangeEnd, QList<QStringList>* docId)
{
    Firebird fb;
    QString statement;

    if (rangeBegin=="0" && rangeEnd=="all") {
        statement = "SELECT vk_id,owner_id,title FROM vkdoc";
        fb.query(statement, docId);
        return;
    }
    if (rangeEnd=="") {
        statement = "SELECT vk_id,owner_id,title FROM vkdoc WHERE id = " + rangeBegin;
        fb.query(statement, docId);
        return;
    }
    if (rangeBegin != "", rangeEnd != "") {
        statement = "SELECT vk_id,owner_id,title FROM vkdoc WHERE id BETWEEN "+rangeBegin+" AND "+rangeEnd;
        fb.query(statement, docId);
        return;
    }
}

bool Firecontrol::postExist(QString& postId)
{
    Firebird fb;
    QList<int> index;

    QString statement = "SELECT id FROM vkpost WHERE vk_id = " + postId;
    qDebug() << statement;
    fb.query(statement, &index);
    qDebug() << "index.size:" << index.size();
    return index.size() > 0 ? true : false;
}

