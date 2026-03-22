#include "profilewidget.h"
#include "ui_profilewidget.h"
#include "Database/database.h"
#include "Utils/timeutils.h"
#include "Session/session.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QPixmap>

ProfileWidget::ProfileWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileWidget)
{
    ui->setupUi(this);

    ui->labelName->setStyleSheet("font-size:34px; font-weight:600;");
    ui->labelMail->setStyleSheet("font-size:16px; color:#9e9e9e;");
    ui->labelTotalForWeek->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelTotalForMonth->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelMoneyForMonth->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelMoneyForWeek->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelTextForMoneyEdit->setStyleSheet("font-size:16px; color:#9e9e9e;");
    ui->spinBoxMoneyForH->setValue(Session::instance().moneyForHour());
    connect(ui->spinBoxMoneyForH, &QSpinBox::valueChanged, this, [this](){
        updateMoneyForHour();
    });

    refresh();
}

ProfileWidget::~ProfileWidget()
{
    delete ui;
}

void ProfileWidget::updateMoneyForHour(){

    int user_id = Session::instance().userId();
    if(user_id == -1) return;
    int moneyForHour = ui->spinBoxMoneyForH->value();

    QSqlQuery query(Database::instance().db);
    query.prepare("UPDATE users SET money_for_hour = ? WHERE user_id = ? AND is_active = ?");
    query.addBindValue(moneyForHour);
    query.addBindValue(user_id);
    query.addBindValue(true);

    if(!query.exec()){
        qDebug() << "Ошибка запроса: " << query.lastError().text();
        return;
    }

    Session::instance().setMoneyForHour(moneyForHour);

    refresh();
}

void ProfileWidget::refresh()
{
    ui->labelName->setText(Session::instance().name());
    ui->labelMail->setText(Session::instance().mail());
    ui->labelAvatar->setText("");
    ui->labelAvatar->setAlignment(Qt::AlignCenter);

    QPixmap pix(Session::instance().photo());
    if (!pix.isNull()) {
        ui->labelAvatar->setScaledContents(true);
        ui->labelAvatar->setPixmap(pix);
    }

    int userId = Session::instance().userId();
    if (userId == -1) {
        ui->labelTotalForWeek->setText("Отработано за неделю: -");
        ui->labelTotalForMonth->setText("Отработано за месяц: -");
        return;
    }

    int weekMinutes = fetchMinutesByQuery(
        "SELECT COALESCE(SUM(EXTRACT(EPOCH FROM (end_time - start_time)))/60, 0)::int "
        "FROM work_intervals "
        "WHERE user_id = ? AND is_active = ? "
        "AND work_date >= date_trunc('week', CURRENT_DATE)::date "
        "AND work_date <  (date_trunc('week', CURRENT_DATE) + interval '7 days')::date",
        userId,
        true
        );

    int monthMinutes = fetchMinutesByQuery(
        "SELECT COALESCE(SUM(EXTRACT(EPOCH FROM (end_time - start_time)))/60, 0)::int "
        "FROM work_intervals "
        "WHERE user_id = ? AND is_active = ? "
        "AND work_date >= date_trunc('month', CURRENT_DATE)::date "
        "AND work_date <  (date_trunc('month', CURRENT_DATE) + interval '1 month')::date",
        userId,
        true
        );

    QString weekStr = TimeUtils::formatWorkedRu(weekMinutes);
    QString monthStr = TimeUtils::formatWorkedRu(monthMinutes);
    int weekMoney = Session::instance().moneyForHour() * (weekMinutes/60);
    int monthMoney = Session::instance().moneyForHour() * (monthMinutes/60);
    ui->labelMoneyForWeek->setText("Заработано за неделю: " + QString("%1").arg(weekMoney));
    ui->labelMoneyForMonth->setText("Заработано за месяц: " + QString("%1").arg(monthMoney));
    ui->labelTotalForWeek->setText("Отработано за неделю: " + (weekStr.isEmpty() ? "0 минут" : weekStr));
    ui->labelTotalForMonth->setText("Отработано за месяц: " + (monthStr.isEmpty() ? "0 минут" : monthStr));
}

void ProfileWidget::on_pushButtonToCalendar_clicked()
{
    emit openCalendar();
}

void ProfileWidget::on_pushButtonQuit_clicked()
{
    emit quitApp();
}

int ProfileWidget::fetchMinutesByQuery(const QString &sql, int userId, bool isActive)
{
    QSqlQuery q(Database::instance().db);
    q.prepare(sql);
    q.addBindValue(userId);
    q.addBindValue(isActive);

    if (!q.exec()) {
        qDebug() << "Ошибка при получении рабочих минут профиля:" << q.lastError().text();
        return 0;
    }

    if (!q.next())
        return 0;

    return q.value(0).toInt();
}
