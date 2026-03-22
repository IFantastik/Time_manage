#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QWidget>
#include <QDate>
#include <QMap>
#include "qpushbutton.h"

namespace Ui {
class CalendarWidget;
}

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);
    ~CalendarWidget();

signals:
    void openProfile();
    void openStatistic();

private slots:
    void on_pushButtonToProfile_clicked();
    void on_pushButtonAddData_clicked();

    void onDayClicked();
    void onMonthYearChanged();

    void on_pushButtonDeleteData_clicked();

    void on_pushButtonToStat_clicked();

    void on_pushButtonSaveInFile_clicked();

private:
    Ui::CalendarWidget *ui;

    void rebuildCalendarGrid();
    void buildWeekHeader();
    void clearGridLayout(QLayout *layout);

    void loadWorkedMinutesForMonth(int year, int month);
    QMap<QDate,int> workedMinutesByDate;
    QMap<QDate,bool> hasRecordsByDate;

    void applyDayStyle(QPushButton *btn, bool hasData);
    static QString pluralRu(int typeTime, const QString& one, const QString& few, const QString& many);
};


#endif // CALENDARWIDGET_H
