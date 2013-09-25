#ifndef APIMETHODEXECUTOR_H
#define APIMETHODEXECUTOR_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include "connection/customnetworkmanager.h"

class ApiMethodExecutor : public QObject
{
    Q_OBJECT

public:
    explicit ApiMethodExecutor(QString token, QObject *parent = 0);
    QJsonObject executeMethod(QString methodName, QMap<QString,QString> params);
    ~ApiMethodExecutor();
signals:
    void networkStatus(bool isOk);
private:
    CustomNetworkAccessManager *networkAccessManager;
    QString token;
    volatile unsigned int timeoutCounter;
};
#endif // APIMETHODEXECUTOR_H

