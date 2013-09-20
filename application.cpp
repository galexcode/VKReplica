#include "application.h"
#include "contactmodel.h"
#include "mainwindow.h"
#include "auth.h"
#include <QDebug>

#include <QApplication>
#include <QStandardPaths>
#include <QWidget>

#include <QMap>
#include <QString>
#include "apimethodexecutor.h"
#include "longpollexecutor.h"
#include "application.h"
#include "dialog.h"
#include "mainwindow.h"

#include <QJsonObject>

Application::Application(QObject *parent) : QObject(parent) {
}


void Application::exec() {
    auth = new Auth(this);
    connect(auth, SIGNAL(authStatusChanged(bool)), this, SLOT(onAuthStatusChanged(bool)));
    auth->showAuthDialog();

    contactModel = new ContactModel(this,this);
    mainWindow = new MainWindow(this, contactModel);
    mainWindow->show();
}

void Application::applyUser() {
    QMap<QString,QString> params;
    params.insert("user_ids",auth->getUserId());

    QJsonObject userJson = apiMethodExecutor->executeMethod("users.get",params);
    QVariantMap user = userJson.value("response").toVariant().toList().value(0).toMap();
    this->userDisplayName = user.value("first_name").toString() + " " + user.value("last_name").toString();
    this->userId = auth->getUserId();
}

void Application::onAuthStatusChanged(bool isLogin) {
    if (isLogin) {
        apiMethodExecutor = new ApiMethodExecutor(auth->getToken(),this);
        longPollExecutor = new LongPollExecutor(this,this);
        applyUser();
    } else {
        apiMethodExecutor->deleteLater();
        longPollExecutor->deleteLater();
        userDisplayName = QString();
        userId = QString();
    }
    mainWindow->applyAuthStatus(isLogin);
}

Application::~Application() {

}

