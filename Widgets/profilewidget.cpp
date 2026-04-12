#include "profilewidget.h"
#include "ui_profilewidget.h"
#include "Database/database.h"
#include "Utils/timeutils.h"
#include "Session/session.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QPixmap>
#include <QMessageBox>

ProfileWidget::ProfileWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileWidget)
{
    ui->setupUi(this);
    ui->label_error->clear();
    ui->label_error->setStyleSheet("color: red;");
    ui->le_money->setVisible(false);
    ui->le_prof->setVisible(false);
    ui->widget->setVisible(false);
    ui->labelName->setStyleSheet("font-size:34px; font-weight:600;");
    ui->labelMail->setStyleSheet("font-size:16px; color:#9e9e9e;");
    ui->labelTotalForWeek->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelTotalForMonth->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelMoneyForMonth->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelMoneyForWeek->setStyleSheet("font-size:20px; font-weight:600;");
    ui->labelTextForMoneyEdit->setStyleSheet("font-size:16px; color:#9e9e9e;");
    ui->spinBoxMoneyForH->setValue(Session::instance().moneyForHour());
    setProfession();
    connect(ui->spinBoxMoneyForH, &QSpinBox::valueChanged, this, [this](){
        updateMoneyForHour();
    });
    connect(ui->cb_profession, &QComboBox::currentIndexChanged, this , &ProfileWidget::saveCbChange);
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
    query.prepare("UPDATE users SET money_for_hour = ? WHERE user_id = ?");
    query.addBindValue(moneyForHour);
    query.addBindValue(user_id);

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

void ProfileWidget::visibleAddProf(bool status)
{
    ui->le_money->setVisible(status);
    ui->le_prof->setVisible(status);
    ui->widget->setVisible(status);
    ui->label_proffesion->setVisible(!status);
    ui->cb_profession->setVisible(!status);
    ui->pb_showadd->setVisible(!status);
}

void ProfileWidget::on_pb_showadd_clicked()
{
    ui->label_error->clear();
    visibleAddProf(true);
}


void ProfileWidget::on_pb_add_clicked()
{
    ui->label_error->clear();

    QString professionName = ui->le_prof->text().trimmed();
    int moneyForHour = ui->le_money->text().trimmed().toInt();

    if (professionName.isEmpty()) {
        ui->label_error->setText("Введите название профессии");
        return;
    }

    if (moneyForHour <= 0) {
        ui->label_error->setText("Введите корректную ставку");
        return;
    }

    QSqlQuery checkQuery(Database::instance().db);
    checkQuery.prepare("SELECT id FROM profession WHERE name = ?");
    checkQuery.addBindValue(professionName);

    if (!checkQuery.exec()) {
        qDebug() << "Ошибка проверки профессии:" << checkQuery.lastError().text();
        return;
    }

    if (checkQuery.next()) {
        ui->label_error->setText("Такая профессия уже существует");
        return;
    }

    QSqlQuery query(Database::instance().db);
    query.prepare("INSERT INTO profession (name, money_for_hour) VALUES (?, ?)");
    query.addBindValue(professionName);
    query.addBindValue(moneyForHour);

    if (!query.exec()) {
        qDebug() << "Ошибка добавления профессии:" << query.lastError().text();
        return;
    }

    ui->le_prof->clear();
    ui->le_money->clear();
    ui->label_error->clear();

    setProfession();
    visibleAddProf(false);
}

void ProfileWidget::on_pb_exit_clicked()
{
    ui->label_error->clear();
    visibleAddProf(false);
}

void ProfileWidget::setProfession()
{
    ui->cb_profession->clear();
    professions.clear();
    ui->cb_profession->addItem("", QVariant());

    QSqlQuery query(Database::instance().db);
    query.prepare("SELECT id, name, money_for_hour FROM profession");

    if (!query.exec()) {
        qDebug() << "Ошибка получения профессий:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        Profession prof;
        prof.id = query.value(0).toInt();
        prof.name = query.value(1).toString();
        prof.moneyForHour = query.value(2).toInt();

        professions.append(prof);
        ui->cb_profession->addItem(prof.name, prof.id);
    }

    int professionId = Session::instance().profession_id();

    if (professionId == -1) {
        ui->cb_profession->setCurrentIndex(0);
        ui->spinBoxMoneyForH->setEnabled(true);
        return;
    }

    int index = ui->cb_profession->findData(professionId);
    if (index != -1) {
        ui->cb_profession->setCurrentIndex(index);

        for (const Profession &prof : professions) {
            if (prof.id == professionId) {
                ui->spinBoxMoneyForH->blockSignals(true);
                ui->spinBoxMoneyForH->setValue(prof.moneyForHour);
                ui->spinBoxMoneyForH->blockSignals(false);
                ui->spinBoxMoneyForH->setEnabled(false);
                break;
            }
        }
    }
}

void ProfileWidget::saveCbChange()
{
    int userId = Session::instance().userId();
    if (userId == -1)
        return;

    QVariant data = ui->cb_profession->currentData();

    if (data.isNull() || !data.isValid()) {
        QSqlQuery query(Database::instance().db);
        query.prepare("UPDATE users SET profession_id = NULL, money_for_hour = ? WHERE user_id = ?");
        query.addBindValue(ui->spinBoxMoneyForH->value());
        query.addBindValue(userId);

        if (!query.exec()) {
            qDebug() << "Ошибка обновления profession_id:" << query.lastError().text();
            return;
        }

        Session::instance().setProfessionId(-1);
        ui->spinBoxMoneyForH->setEnabled(true);

        refresh();
        return;
    }

    int professionId = data.toInt();

    for (const Profession &prof : professions) {
        if (prof.id == professionId) {
            QSqlQuery query(Database::instance().db);
            query.prepare("UPDATE users SET profession_id = ?, money_for_hour = ? WHERE user_id = ?");
            query.addBindValue(prof.id);
            query.addBindValue(prof.moneyForHour);
            query.addBindValue(userId);

            if (!query.exec()) {
                qDebug() << "Ошибка обновления profession_id:" << query.lastError().text();
                return;
            }

            Session::instance().setProfessionId(prof.id);
            Session::instance().setMoneyForHour(prof.moneyForHour);

            ui->spinBoxMoneyForH->blockSignals(true);
            ui->spinBoxMoneyForH->setValue(prof.moneyForHour);
            ui->spinBoxMoneyForH->blockSignals(false);
            ui->spinBoxMoneyForH->setEnabled(false);

            refresh();
            return;
        }
    }
}

void ProfileWidget::on_pb_deleteProf_clicked()
{
    ui->label_error->clear();

    QVariant data = ui->cb_profession->currentData();
    if (!data.isValid() || data.isNull()) {
        ui->label_error->setText("Выберите профессию для удаления");
        return;
    }

    int professionId = data.toInt();
    int userId = Session::instance().userId();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Удаление профессии",
        "Вы уверены, что хотите удалить профессию?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    QSqlQuery checkQuery(Database::instance().db);
    checkQuery.prepare("SELECT COUNT(*) FROM users WHERE profession_id = ? AND user_id <> ?");
    checkQuery.addBindValue(professionId);
    checkQuery.addBindValue(userId);

    if (!checkQuery.exec()) {
        qDebug() << "Ошибка проверки пользователей:" << checkQuery.lastError().text();
        return;
    }

    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        ui->label_error->setText("Нельзя удалить: у других пользователей выбрана эта профессия");
        return;
    }

    QSqlQuery updateUserQuery(Database::instance().db);
    updateUserQuery.prepare("UPDATE users SET profession_id = NULL WHERE user_id = ?");
    updateUserQuery.addBindValue(userId);

    if (!updateUserQuery.exec()) {
        qDebug() << "Ошибка обновления пользователя:" << updateUserQuery.lastError().text();
        return;
    }

    QSqlQuery deleteQuery(Database::instance().db);
    deleteQuery.prepare("DELETE FROM profession WHERE id = ?");
    deleteQuery.addBindValue(professionId);

    if (!deleteQuery.exec()) {
        qDebug() << "Ошибка удаления профессии:" << deleteQuery.lastError().text();
        return;
    }

    Session::instance().setProfessionId(-1);

    ui->cb_profession->setCurrentIndex(0);
    ui->spinBoxMoneyForH->setEnabled(true);

    setProfession();
    refresh();

    ui->label_error->clear();
}
