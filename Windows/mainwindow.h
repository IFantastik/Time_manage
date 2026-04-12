#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Widgets/profilewidget.h"
#include "Widgets/calendarwidget.h"
#include "Widgets/statistic.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:

private:
    Ui::MainWindow *ui;
    ProfileWidget *profilePage;
    CalendarWidget *calendarPage;
    Statistic *statisticPage;
    void createPages();
    void destroyPages();
    void setupConnections();
};
#endif // MAINWINDOW_H
