#ifndef EDITACCOUNTDIALOG_H
#define EDITACCOUNTDIALOG_H

#include <QDialog>

namespace Ui {
class EditAccountDialog;
}

class EditAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditAccountDialog(QWidget *parent = nullptr);
    ~EditAccountDialog();

private:
    Ui::EditAccountDialog *ui;
};

#endif // EDITACCOUNTDIALOG_H
