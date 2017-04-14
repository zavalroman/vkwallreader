#ifndef FIRECONTROL_H
#define FIRECONTROL_H

#include <QObject>

#include "../ibpp/tests/C++/qt-firebird.h"
#include "vkpost.h"

class Firecontrol : public QObject
{
    Q_OBJECT
public:
    explicit Firecontrol(QObject *parent = 0);

    void getDocId(QString rangeBegin, QString rangeEnd, QList<QStringList> *docId);
    bool postExist(QString& postId);
    void textPrepare(QString *text) const;
signals:

public slots:
    void vkpostToDb(Vkpost*);

private:

};

#endif // FIRECONTROL_H
