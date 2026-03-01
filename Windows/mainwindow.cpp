#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Session/session.h"
#include "Dialogs/loginwindow.h"
#include "Widgets/statistic.h"

MainWindow::MainWindow()
    : QMainWindow()
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/icons/appicon.png"));

    profilePage = new ProfileWidget(this);
    calendarPage = new CalendarWidget(this);
    statisticPage = new Statistic(this);

    ui->stackedWidget->addWidget(profilePage);
    ui->stackedWidget->addWidget(calendarPage);
    ui->stackedWidget->addWidget(statisticPage);

    ui->stackedWidget->setCurrentWidget(calendarPage);

    connect(calendarPage, &CalendarWidget::openProfile,this,[this](){
        profilePage->refresh();
        ui->stackedWidget->setCurrentWidget(profilePage);
    });

    connect(calendarPage, &CalendarWidget::openStatistic,this,[this](){
        ui->stackedWidget->setCurrentWidget(statisticPage);
    });

    connect(statisticPage,&Statistic::openCalendar,this,[this](){
        ui->stackedWidget->setCurrentWidget(calendarPage);
    });

    connect(profilePage, &ProfileWidget::openCalendar,this,[this](){
        ui->stackedWidget->setCurrentWidget(calendarPage);
    });

    connect(profilePage, &ProfileWidget::quitApp,this,[this](){
        Session::instance().clear();
        this->hide();
        loginwindow login;
        if(login.exec() == QDialog::Accepted){
            this->show();
            ui->stackedWidget->setCurrentWidget(calendarPage);
        }else{
            qApp->quit();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

