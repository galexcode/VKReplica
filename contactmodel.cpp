#include "contactmodel.h"
#include "application.h"
#include <QApplication>
#include <QJsonObject>
#include "longpollexecutor.h"

bool ascDisplayName(const Contact* c1 , const Contact* c2 ) {
    if (c1->hasUnreadMessage && !c2->hasUnreadMessage) {
        return true;
    } else if (!c1->hasUnreadMessage && c2->hasUnreadMessage) {
        return false;
    } //Контакты, от которых есть непрочитанные сообщения в любом случае попадают наверх
    if (c1->isOnline && !c2->isOnline) {
        return true;
    } else if ((c1->isOnline && c2->isOnline) || (!c1->isOnline && !c2->isOnline)) {
        if (c1->displayName < c2->displayName) {
            return true;
        }
    }
    return false;
}

bool descRating(const Contact* c1 , const Contact* c2 ) {
    if (c1->hasUnreadMessage && !c2->hasUnreadMessage) {
        return true;
    } else if (!c1->hasUnreadMessage && c2->hasUnreadMessage) {
        return false;
    } //Контакты, от которых есть непрочитанные сообщения в любом случае попадают наверх
    if (c1->isOnline && !c2->isOnline) {
        return true;
    } else if ((c1->isOnline && c2->isOnline) || (!c1->isOnline && !c2->isOnline)) {
        if (c1->rating > c2->rating) {
            return true;
        }
    }
    return false;

}

ContactModel::ContactModel(const Application* application, QObject* parent) :
    QAbstractListModel(parent),
    application(application),
    allVisible(true)
{
    contactList = QList<Contact*>();
    contactStorage = QList<Contact*>();
    unreadSet = QSet<QString>();
}

void ContactModel::load() {
    QMap<QString, QString> map = QMap<QString, QString>();
    map.insert("order", "hints");
    map.insert("fields","online");
    QJsonObject result = application->getApiMethodExecutor().executeMethod("friends.get", map);

    QVariantList contactJsonList = result.toVariantMap().take("response").toMap().take("items").toList();

    //Заполнение списка контактов и списка порядка
    contactList.clear();
    if (!contactStorage.isEmpty()) {
        qDeleteAll(contactStorage);
        contactStorage.clear();

    }
    int rating = contactJsonList.size();
    foreach (QVariant value,contactJsonList) {
        QMap<QString, QVariant> valueMap = value.toMap();
        bool isOnline = valueMap.value("online").toDouble();

        QString userId = QString::number(
            static_cast< int >(
                (double)valueMap.value("id").toDouble()
            )
        );
        QString displayName =
                valueMap.value("first_name").toString()
                .append(" ")
                .append(valueMap.value("last_name").toString());

        Contact* contact = new Contact{userId,rating,displayName,isOnline,false};
        rating--;
        contactStorage.push_back(contact);
        if (allVisible || isOnline) {
           contactList.push_back(contact);
        }
    }

    checkUnreadMessages();

    sort();
    connect(&application->getLongPollExecutor(),SIGNAL(contactIsOnline(QString,bool)),this, SLOT(setContactOnline(QString,bool)));
    connect(&application->getLongPollExecutor(),SIGNAL(messageRecieved(QString,bool)),this, SLOT(acceptUnreadMessage(QString,bool)));
}

void ContactModel::sort() {
    switch (sortOrder) {
        case DescRating: {
            qSort(contactList.begin(),contactList.end(),descRating);
            break;
        }
        case AscDisplayName:{
            qSort(contactList.begin(),contactList.end(),ascDisplayName);
            break;
        }
    }
}

void ContactModel::reloadFromStorage() {
    contactList.clear();
    foreach (Contact* contact, contactStorage) {
        if (allVisible || contact->isOnline) {
            contactList.push_back(contact);
        }
    }
    sort();
}


/*Выгрузка контактов*/
void ContactModel::unload() {
    contactList.clear();
    if (!contactStorage.isEmpty()) {
        qDeleteAll(contactStorage);
        contactStorage.clear();

    }
    disconnect(&application->getLongPollExecutor(),SIGNAL(contactIsOnline(QString,bool)),this, SLOT(setContactOnline(QString,bool)));
    disconnect(&application->getLongPollExecutor(),SIGNAL(messageRecieved(QString,bool)),this, SLOT(acceptUnreadMessage(QString,bool)));
}

void ContactModel::checkUnreadMessages() {
    QMap<QString,QString> params;
    params.insert("filters","1");
    QJsonObject result = application->getApiMethodExecutor().executeMethod("messages.get",params);

    QVariantList unreadMessageList = result.toVariantMap().take("response").toMap().take("items").toList();
    foreach (QVariant unreadMessage, unreadMessageList) {
        QString userId = unreadMessage.toMap().value("user_id").toString();
        Contact* contact = findByUserId(userId);
        if (contact != nullptr) {
            contact->hasUnreadMessage = true;
        }
        unreadSet.insert(userId);
    }
    if (!unreadSet.isEmpty()) {
        emit hasUnreadMessage(true);
    }
}

int ContactModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return contactList.count();
}

QVariant ContactModel::data(const QModelIndex& index, int role) const {
    if (index.row() < 0 || index.row() >= contactList.size()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        Contact* contact = contactList.at(index.row());
        QVariant contactVariant;
        contactVariant.setValue(contact);
        return contactVariant;
    }
    return QVariant();
}


bool ContactModel::insertRows(int row, int count, const QModelIndex &parent) {
    if (count < 1 || row < 0 || row > rowCount(parent)) {
        return false;
    }
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int r = 0; r < count; ++r) {
        Contact* emptyContact = new Contact();
        contactList.insert(row, emptyContact);
    }
    endInsertRows();
    return true;
}

bool ContactModel::removeRows(int row, int count, const QModelIndex &parent) {
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)) {
       return false;
    }
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int r = 0; r < count; ++r) {
        contactList.removeAt(row);
    }
    endRemoveRows();
    return true;
}

Contact* ContactModel::getByRow(int row) const {
    if (row<0 || row>=contactList.size()) {
        return NULL;
    }
    return contactList.at(row);
}

Contact* ContactModel::findByUserId(const QString& userId) const {
    foreach (Contact* contact, contactStorage) {
        if (contact->userId == userId) {
            return contact;
        }
    }
    //        TODO Если контакт не найден, попробовать сделать запрос по API,
    //        и только если там не найдется, выходить из метода
    return nullptr;
}

void ContactModel::applyContactsVisibility(const bool allVisible) {
    this->allVisible = allVisible;
    reloadFromStorage();
    emit dataChanged(index(0), index(contactList.size()-1));
}


void ContactModel::push(Contact* contact) {
    if (contact->hasUnreadMessage) {
        contactList.push_front(contact); //Контакты, от которых есть непрочитанные сообщения в любом случае попадают наверх
        return;
    } else if (allVisible || contact->isOnline) {
        Contact* lastOnline;
        Contact* lastOffline;
        switch(sortOrder) {
            case DescRating: {
                foreach (Contact* sortedContact, contactList) {
                    if (!sortedContact->hasUnreadMessage && sortedContact->rating <= contact->rating) {
                        if ((sortedContact->isOnline && contact->isOnline) || (!sortedContact->isOnline && !contact->isOnline)) {
                            contactList.insert(contactList.indexOf(sortedContact),contact);
                            return;
                        }
                    }
                    if (sortedContact->isOnline) {
                        lastOnline = sortedContact;
                    } else {
                        lastOffline = sortedContact;
                    }

                }
                break;
            }
            case AscDisplayName: {
                foreach (Contact* sortedContact, contactList) {
                    if (sortedContact->displayName > contact->displayName) {
                        if ((sortedContact->isOnline && !sortedContact->hasUnreadMessage && contact->isOnline) || (!sortedContact->isOnline && !contact->isOnline)) {
                            contactList.insert(contactList.indexOf(sortedContact),contact);
                            return;
                        }
                    }
                    if (sortedContact->isOnline) {
                        lastOnline = sortedContact;
                    } else {
                        lastOffline = sortedContact;
                    }
                }
                break;
            }
        }
        //Добавляем контакт в конец списка, если так никуда и не добавились ранее
        if (contact->isOnline) {
            contactList.insert(contactList.indexOf(lastOnline)+1,contact);
        } else {
            contactList.insert(contactList.indexOf(lastOffline)+1,contact);
        }
    }
}

void ContactModel::insert(Contact* contact, int idx) {
    if (idx < 0) {
        push(contact);
    } else {
        contactList.insert(idx,contact);
    }
    emit dataChanged(index(0), index(contactList.size()-1));
}

void ContactModel::refreshContact(Contact* contact) {
    if (contactList.contains(contact)) {
        contactList.removeAt(contactList.indexOf(contact));
    }
    this->push(contact);

    emit dataChanged(index(0), index(contactList.size()-1));
}

void ContactModel::setContactOnline(QString userId, bool isOnline){
    Contact* contact = findByUserId(userId);
    if (contact == nullptr) {
        return;
    }

    if (contact->isOnline != isOnline) {
        contact->isOnline = isOnline;
        refreshContact(contact);
    }
}

void ContactModel::acceptUnreadMessage(QString userId, bool hasUnread) {
    Contact* contact = findByUserId(userId);
    if (contact == nullptr) {
        return;
    }

    if (hasUnread) {
        if (unreadSet.isEmpty()) {
            emit hasUnreadMessage(true);
        }
        unreadSet.insert(userId);
    } else {
        unreadSet.remove(userId);
    }
    if (unreadSet.isEmpty()) {
        emit hasUnreadMessage(false);
    }

    if (contact->hasUnreadMessage != hasUnread) {
        contact->hasUnreadMessage = hasUnread;
        refreshContact(contact);
    }
}

