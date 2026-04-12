#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include <QShowEvent>

namespace Ui {
class ProfileWidget;
}

struct Profession
{
    int id;
    QString name;
    int moneyForHour;
};

class ProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget *parent = nullptr);
    ~ProfileWidget();
    void refresh();

signals:
    void openCalendar();
    void quitApp();

private slots:
    void on_pushButtonToCalendar_clicked();
    void on_pushButtonQuit_clicked();
    void updateMoneyForHour();

    void on_pb_showadd_clicked();

    void on_pb_add_clicked();

    void on_pb_exit_clicked();

    void saveCbChange();

    void on_pb_deleteProf_clicked();

private:
    Ui::ProfileWidget *ui;
    QVector<Profession> professions;
    void visibleAddProf(bool status);
    void setProfession();
    static int fetchMinutesByQuery(const QString &sql, int userId, bool isActive);

};

#endif // PROFILEWIDGET_H
