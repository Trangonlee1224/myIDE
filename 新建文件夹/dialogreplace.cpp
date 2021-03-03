#include "dialogreplace.h"
#include "ui_dialogreplace.h"

DialogReplace::DialogReplace(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogReplace)
{
    ui->setupUi(this);
}

DialogReplace::~DialogReplace()
{
    delete ui;
}
