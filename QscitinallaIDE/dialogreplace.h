#ifndef DIALOGREPLACE_H
#define DIALOGREPLACE_H

#include <QDialog>

namespace Ui {
class DialogReplace;
}

class DialogReplace : public QDialog
{
    Q_OBJECT

public:
    explicit DialogReplace(QWidget *parent = nullptr);
    ~DialogReplace();

private:
    Ui::DialogReplace *ui;
};

#endif // DIALOGREPLACE_H
