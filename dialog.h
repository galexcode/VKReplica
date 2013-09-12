#ifndef DIALOG_H
#define DIALOG_H

#include "application.h"

#include <QWidget>

namespace Ui {
class Dialog;
}

class Dialog : public QWidget
{
    Q_OBJECT

public:
    explicit Dialog(Application *application, QString userId, QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    Application *application;
    QString userId;

    void configSplitter();
};

#endif // DIALOG_H
