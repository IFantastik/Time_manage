#include "addinterval.h"
#include "ui_addinterval.h"
#include "Session/session.h"
#include "Database/database.h"
#include "QSqlQuery"
#include "QDebug"
#include "QSqlError"
#include "QTime"
#include "QString"

addInterval::addInterval(const QDate &date, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addInterval),
    m_date(date)
{
    ui->setupUi(this);

    ui->dateEdit->setDate(m_date);
    ui->dateEdit->setCalendarPopup(true);

    ui->pushButtonEdit->setEnabled(false);
    ui->pushButtonDelete->setEnabled(false);
    ui->tableIntervals->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableIntervals->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableIntervals->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->checkBox->setCheckState(Qt::CheckState::Checked);
    connect(ui->dateEdit,&QDateEdit::dateChanged,this,[this](const QDate &date){
        m_date = date;
        loadIntervals();
    });

    loadIntervals();
}

addInterval::~addInterval()
{
    delete ui;
}

void addInterval::loadIntervals(){
    ui->tableIntervals->setRowCount(0);

    int userId = Session::instance().userId();
    if(userId == -1) return;
    QSqlQuery query(Database::instance().db);
    query.prepare("SELECT id, start_time, end_time, note, is_active "
                  "FROM work_intervals "
                  "WHERE user_id = ? AND work_date = ? "
                  "ORDER BY start_time");
    query.addBindValue(userId);
    query.addBindValue(m_date);

    if(!query.exec()){
        ui->label_error->setText("Ошибка загрузки данных: " + query.lastError().text());
        return;
    }

    int row = 0;

    while(query.next()){
        int id = query.value("id").toInt();
        QTime start = query.value("start_time").toTime();
        QTime end = query.value("end_time").toTime();
        QString note = query.value("note").toString();
        bool isActive = query.value("is_active").toBool();
        ui->tableIntervals->insertRow(row);

        auto *itemStart = new QTableWidgetItem(start.toString("HH:mm"));
        auto *itemEnd = new QTableWidgetItem(end.toString("HH:mm"));
        auto *itemNote = new QTableWidgetItem(note);

        itemStart->setData(Qt::UserRole, id);
        itemStart->setData(Qt::UserRole + 1, isActive);

        ui->tableIntervals->setItem(row, 0, itemStart);
        ui->tableIntervals->setItem(row, 1, itemEnd);
        ui->tableIntervals->setItem(row, 2, itemNote);

        row++;
    }
}

bool addInterval::checkTime(int userId, const QDate &date, const QTime &start, const QTime &end, int excludeId){

    QSqlQuery check(Database::instance().db);
    check.prepare(
        "SELECT 1 "
        "FROM work_intervals "
        "WHERE user_id = ? AND work_date = ? "
        "AND id <> ? "
        "AND (? < end_time) AND (? > start_time) "
        "LIMIT 1"
        );
    check.addBindValue(userId);
    check.addBindValue(date);
    check.addBindValue(excludeId);
    check.addBindValue(start);
    check.addBindValue(end);

    if (!check.exec()){
        ui->label_error->setText("Ошибка проверки:" + check.lastError().text());
        return false;
    }
    if (check.next()){
        ui->label_error->setText("Интервал пересекается с уже существующей записью");
        return false;
    }
    return true;
}

void addInterval::on_pushButtonAdd_clicked()
{
    ui->label_error->clear();

    int userId = Session::instance().userId();
    if(userId == -1) return;

    QTime start = ui->timeEditStart->time();
    QTime end = ui->timeEditEnd->time();
    QString note = ui->TextEditNote->toPlainText();
    bool isActive = ui->checkBox->isChecked();
    if(std::size(note)>200){
        ui->label_error->setText(QString("Слишком длинная заметка: %1/200").arg(note.size()));
        return;
    }
    if(end <= start){
        ui->label_error->setText("Время окончания меньше времени начала");
        return;
    }

    if (!checkTime(userId, m_date, start, end, -1)){
        return;
    }

    QSqlQuery query(Database::instance().db);
    query.prepare("INSERT INTO work_intervals(user_id, work_date, start_time, end_time, note, is_active)"
              " VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(m_date);
    query.addBindValue(start);
    query.addBindValue(end);
    query.addBindValue(note);
    query.addBindValue(isActive);

    if (!query.exec()){
        ui->label_error->setText("Ошибка добавления: " + query.lastError().text());
        return;
    }
    m_selectedId = -1;

    ui->tableIntervals->clearSelection();
    ui->TextEditNote->clear();

    loadIntervals();
}

void addInterval::on_pushButtonDelete_clicked()
{
    ui->label_error->clear();

    if(m_selectedId == -1) return;
    QSqlQuery query(Database::instance().db);
    query.prepare("DELETE FROM work_intervals WHERE id = ?");
    query.addBindValue(m_selectedId);

    if(!query.exec()){
        ui->label_error->setText("Ошибка: " + query.lastError().text());
        return;
    }

    ui->tableIntervals->clearSelection();
    ui->TextEditNote->clear();
    loadIntervals();
}


void addInterval::on_tableIntervals_itemSelectionChanged()
{
    ui->label_error->clear();

    auto items = ui->tableIntervals->selectedItems();

    if(items.isEmpty()){
        ui->pushButtonDelete->setEnabled(false);
        ui->pushButtonEdit->setEnabled(false);
        ui->TextEditNote->clear();
        ui->checkBox->setChecked(true);
        m_selectedId = -1;
        return;
    }

    int row = ui->tableIntervals->currentRow();
    if(row < 0) return;

    auto itemId = ui->tableIntervals->item(row,0);
    if (!itemId) return;
    m_selectedId = itemId->data(Qt::UserRole).toInt();
    bool isActive = itemId->data(Qt::UserRole + 1).toBool();
    QString itemStart = ui->tableIntervals->item(row,0)->text();
    QString itemEnd = ui->tableIntervals->item(row,1)->text();
    QString itemNote = ui->tableIntervals->item(row,2)->text();

    ui->timeEditStart->setTime(QTime::fromString(itemStart,"HH:mm"));
    ui->timeEditEnd->setTime(QTime::fromString(itemEnd,"HH:mm"));
    ui->TextEditNote->setPlainText(itemNote);
    ui->checkBox->setChecked(isActive);

    ui->pushButtonEdit->setEnabled(true);
    ui->pushButtonDelete->setEnabled(true);

}

void addInterval::on_pushButtonEdit_clicked()
{
    ui->label_error->clear();

    if (m_selectedId == -1) return;

    int userId = Session::instance().userId();
    QTime start = ui->timeEditStart->time();
    QTime end = ui->timeEditEnd->time();
    QString note = ui->TextEditNote->toPlainText();
    bool isActive = ui->checkBox->isChecked();

    if (end <= start){
        ui->label_error->setText("Время окончания работы меньше времени начала");
        return;
    }

    if (!checkTime(userId, m_date, start, end, m_selectedId)) return;

    QSqlQuery query(Database::instance().db);
    query.prepare("UPDATE work_intervals "
                  "SET start_time = ?, end_time = ?, note = ?, is_active = ? "
                  "WHERE id = ?");
    query.addBindValue(start);
    query.addBindValue(end);
    query.addBindValue(note);
    query.addBindValue(isActive);
    query.addBindValue(m_selectedId);

    if(!query.exec()){
        ui->label_error->setText("Ошибка: " + query.lastError().text());
        return;
    }

    loadIntervals();
}

