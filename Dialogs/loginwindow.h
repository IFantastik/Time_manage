#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>

namespace Ui {
class loginwindow;
}

class loginwindow : public QDialog
{
    Q_OBJECT

public:
    explicit loginwindow(QDialog *parent = nullptr);
    ~loginwindow();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void addUser(QString login, QString password, QString repeatpas, QString mail, QString name);

    void checkLogin(QString login, QString password);

    void on_pushButtonToJoin_clicked();

    void on_pushButtonReg_clicked();

    void on_pushButtonToReg_clicked();

    void on_pushButtonJoin_clicked();

private:
    Ui::loginwindow *ui;
};

#endif // LOGINWINDOW_H
