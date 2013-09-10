#include "application.h"
#include "mainwindow.h"
#include "auth.h"
#include <QDebug>

#include <QApplication>
#include <QWidget>

#include <QMap>
#include <QString>
#include "apimethodexecutor.h"
#include "longpollexecutor.h"
#include "application.h"
#include "dialog.h"

#include <QJsonObject>

Application::Application(QObject *parent) :
    QObject(parent)
{
}

void Application::exec() {
    auth = new Auth(this);
    auth->exec();

    apiMethodExecutor = new ApiMethodExecutor(auth->getToken(),this);

    MainWindow *mainWindow = new MainWindow(this);
    mainWindow->show();
    longPollExecutor = new LongPollExecutor(this,this);
    mainWindow->applyContactModel(new ContactModel(this,this));
    longPollExecutor->start();

    Dialog* dialog = new Dialog();
    dialog->show();
}

Application::~Application()
{

}

