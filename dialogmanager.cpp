#include "application.h"
#include "dialogmanager.h"
#include "dialogmodel.h"
#include "ui_dialogmanager.h"

DialogManager::DialogManager(Application *application, QWidget *parent) : QDialog(parent), ui(new Ui::DialogManager) {
    this->application = application;
    dialogMap = new QMap<QString,Dialog*>();
    ui->setupUi(this);
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeDialog(int)));
}

DialogManager::~DialogManager() {
    delete ui;
}

void DialogManager::showDialog(Contact* contact) {
    if (!dialogMap->contains(contact->displayName)) {
        Dialog* dialog= new Dialog(application, contact->userId, this);
        DialogModel *dialogModel = new DialogModel(application, contact->userId, this);
        dialog->applyDialogModel(dialogModel);
        dialogMap->insert(contact->displayName,dialog);
        this->ui->tabWidget->addTab(dialog,contact->displayName);
    }
    Dialog* dialog = dialogMap->value(contact->displayName);
    this->show();
    this->activateWindow();
    this->ui->tabWidget->setCurrentWidget(dialog);
}

void DialogManager::closeDialog(int idx) {
    Dialog* dialog = (Dialog*) ui->tabWidget->widget(idx);
    dialogMap->remove(ui->tabWidget->tabText(idx));
    ui->tabWidget->removeTab(idx);
    if (ui->tabWidget->count()<1) {
        this->hide();
    }
}