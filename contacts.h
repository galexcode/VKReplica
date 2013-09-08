#ifndef CONTACTS_H
#define CONTACTS_H

#include <QObject>
#include "application.h"
#include <QList>

struct Contact {
    QString lastName;
    QString firstName;
    bool isOnline;
};

class Contacts : public QObject
{
    Q_OBJECT
public:
    explicit Contacts(Application *application, QObject *parent = 0);
    QList<Contact> getAll() const;

private:
    Application* application;
    QList<Contact> contacts;

signals:

public slots:

};

#endif // CONTACTS_H
