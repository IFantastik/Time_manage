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

    createPages();
    setupConnections();

    ui->stackedWidget->setCurrentWidget(calendarPage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createPages()
{
    profilePage = new ProfileWidget(this);
    calendarPage = new CalendarWidget(this);
    statisticPage = new Statistic(this);

    ui->stackedWidget->addWidget(profilePage);
    ui->stackedWidget->addWidget(calendarPage);
    ui->stackedWidget->addWidget(statisticPage);
}

void MainWindow::destroyPages()
{
    if (profilePage) {
        ui->stackedWidget->removeWidget(profilePage);
        delete profilePage;
        profilePage = nullptr;
    }

    if (calendarPage) {
        ui->stackedWidget->removeWidget(calendarPage);
        delete calendarPage;
        calendarPage = nullptr;
    }

    if (statisticPage) {
        ui->stackedWidget->removeWidget(statisticPage);
        delete statisticPage;
        statisticPage = nullptr;
    }
}

void MainWindow::setupConnections()
{
    connect(calendarPage, &CalendarWidget::openProfile, this, [this]() {
        ui->stackedWidget->setCurrentWidget(profilePage);
    });

    connect(calendarPage, &CalendarWidget::openStatistic, this, [this]() {
        ui->stackedWidget->setCurrentWidget(statisticPage);
    });

    connect(statisticPage, &Statistic::openCalendar, this, [this]() {
        ui->stackedWidget->setCurrentWidget(calendarPage);
    });

    connect(profilePage, &ProfileWidget::openCalendar, this, [this]() {
        ui->stackedWidget->setCurrentWidget(calendarPage);
    });

    connect(profilePage, &ProfileWidget::quitApp, this, [this]() {
        Session::instance().clear();

        destroyPages();

        this->hide();
        loginwindow login;
        if (login.exec() == QDialog::Accepted) {
            createPages();
            setupConnections();
            this->show();
            ui->stackedWidget->setCurrentWidget(calendarPage);
        } else {
            qApp->quit();
        }
    });
}
