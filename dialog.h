#ifndef DIALOG_H
#define DIALOG_H

#include "application.h"
#include "dialogmodel.h"

#include <QWidget>

namespace Ui {
class Dialog;
}

class Dialog : public QWidget
{
    Q_OBJECT

public:
    explicit Dialog(Application *application, QString userId, QWidget *parent = 0);
    void applyDialogModel(DialogModel *dialogModel);
    ~Dialog();

public slots:
    void onFocusTextEdit();
private:
    Ui::Dialog *ui;
    Application *application;
    QString userId;
    DialogModel *dialogModel;

    void configSplitter();

};

#endif // DIALOG_H