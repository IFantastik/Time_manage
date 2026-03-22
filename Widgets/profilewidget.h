#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include <QShowEvent>

namespace Ui {
class ProfileWidget;
}

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

private:
    Ui::ProfileWidget *ui;
    static int fetchMinutesByQuery(const QString &sql, int userId, bool isActive);

};

#endif // PROFILEWIDGET_H
