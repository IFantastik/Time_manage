#include "calendarwidget.h"
#include "ui_calendarwidget.h"
#include "Dialogs/addinterval.h"
#include "Session/session.h"
#include "Database/database.h"
#include "Utils/timeutils.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QLayoutItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

CalendarWidget::CalendarWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalendarWidget)
{
    ui->setupUi(this);

    QDate curDate = QDate::currentDate();
    ui->comboBoxMonth->setCurrentIndex(curDate.month() - 1);
    ui->spinBoxYear->setRange(2000,2100);
    ui->spinBoxYear->setValue(curDate.year());

    connect(ui->comboBoxMonth,&QComboBox::currentIndexChanged,this,[this](int){
        onMonthYearChanged();
    });
    connect(ui->spinBoxYear,&QSpinBox::valueChanged,this,[this](int){
        onMonthYearChanged();
    });

    rebuildCalendarGrid();
}

CalendarWidget::~CalendarWidget()
{
    delete ui;
}

void CalendarWidget::on_pushButtonToProfile_clicked()
{
    emit openProfile();
}

void CalendarWidget::onMonthYearChanged()
{
    rebuildCalendarGrid();
}

void CalendarWidget::on_pushButtonAddData_clicked()
{
    QDate today = QDate::currentDate();
    addInterval dlg(today,this);
    dlg.exec();

    rebuildCalendarGrid();
}

void CalendarWidget::clearGridLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)){
        if (QWidget *w = item->widget()){
            w->deleteLater();
        }
        delete item;
    }
}

void CalendarWidget::buildWeekHeader()
{
    static const QStringList week = {"Пн","Вт","Ср","Чт","Пт","Сб","Вс"};
    for (int col = 0; col < 7 ;col++){
        QLabel *lbl = new QLabel(week[col],this);
        lbl->setAlignment(Qt::AlignHCenter);
        lbl->setStyleSheet("font-weight:600;");
        ui->gridLayoutDays->addWidget(lbl, 0, col);
    }
}

void CalendarWidget::loadWorkedMinutesForMonth(int year, int month)
{
    workedMinutesByDate.clear();
    hasRecordsByDate.clear();

    int user_id = Session::instance().userId();
    if (user_id == -1) return;

    QDate first(year, month, 1);
    QDate last(year, month, first.daysInMonth());

    QSqlQuery query(Database::instance().db);

    query.prepare(
        "SELECT work_date, "
        "COUNT(*) > 0 AS has_any_records, "
        "COALESCE(SUM(EXTRACT(EPOCH FROM (end_time - start_time)) / 60) "
        "FILTER (WHERE is_active = true), 0)::int AS minutes "
        "FROM work_intervals "
        "WHERE user_id = ? AND work_date BETWEEN ? AND ? "
        "GROUP BY work_date"
        );

    query.addBindValue(user_id);
    query.addBindValue(first);
    query.addBindValue(last);

    if (!query.exec()) {
        qDebug() << "loadWorkedMinutesForMonth error:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        QDate d = query.value("work_date").toDate();
        bool hasRecords = query.value("has_any_records").toBool();
        int minutes = query.value("minutes").toInt();

        if (d.isValid()) {
            workedMinutesByDate[d] = minutes;
            hasRecordsByDate[d] = hasRecords;
        }
    }
}

void CalendarWidget::applyDayStyle(QPushButton *btn, bool hasData)
{
    QString style =
        "QPushButton {"
        "  border:1px solid #444;"
        "  border-radius:10px;"
        "}"
        "QPushButton:hover {"
        "  border:1px solid #777;"
        "}"
        "QPushButton:pressed {"
        "  background-color: palette(light);"
        "}";

    if (hasData){
        style =
            "QPushButton {"
            "  border:2px solid orange;"
            "  border-radius:10px;"
            "}"
            "QPushButton:hover {"
            "  border:2px solid #ffb347;"
            "}"
            "QPushButton:pressed {"
            "  background-color: palette(light);"
            "}";
    }

    btn->setStyleSheet(style);
}

void CalendarWidget::rebuildCalendarGrid()
{
    clearGridLayout(ui->gridLayoutDays);
    buildWeekHeader();

    int year = ui->spinBoxYear->value();
    int month = ui->comboBoxMonth->currentIndex() + 1;

    loadWorkedMinutesForMonth(year, month);

    QDate firstDay(year,month,1);
    int daysInMonth = firstDay.daysInMonth();
    int offset = firstDay.dayOfWeek() - 1;

    for(int day = 1;day <= daysInMonth; day++){
        int idx = offset + (day - 1);
        int col = idx % 7;
        int row = 1 + idx / 7;

        QDate date(year, month, day);

        QPushButton *btn = new QPushButton(this);
        btn->setMinimumSize(110,80);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setProperty("date", date);

        btn->setText("");
        btn->setFlat(true);

        int minutes = workedMinutesByDate.value(date, 0);
        bool hasData = (hasRecordsByDate.value(date, false));

        QVBoxLayout *vLayout = new QVBoxLayout(btn);
        vLayout->setContentsMargins(8,8,8,8);
        vLayout->setSpacing(6);

        QHBoxLayout *top = new QHBoxLayout();
        top->setSpacing(6);

        QLabel *lblDayText = new QLabel("Число:", btn);
        lblDayText->setStyleSheet("color:#9e9e9e; font-size:11px;");
        QLabel *lblDayNum = new QLabel(QString::number(day),btn);
        lblDayNum->setStyleSheet("font-size:16px; font-weight:600;");

        top->addWidget(lblDayText);
        top->addWidget(lblDayNum);
        top->addStretch();

        QLabel *lblWorkedText = new QLabel("Отработано:",btn);
        lblWorkedText->setStyleSheet("color:#9e9e9e; font-size:11px;");
        QString workedStr = TimeUtils::formatWorkedRu(minutes);
        QLabel *lblWorkedVal = new QLabel(((minutes > 0) && !workedStr.isEmpty()) ? workedStr : "-", btn);
        lblWorkedVal->setStyleSheet("font-size:12px;");

        vLayout->addLayout(top);
        vLayout->addStretch();
        vLayout->addWidget(lblWorkedText);
        vLayout->addWidget(lblWorkedVal);

        applyDayStyle(btn, hasData);
        connect(btn, &QPushButton::clicked, this, &CalendarWidget::onDayClicked);
        ui->gridLayoutDays->addWidget(btn, row, col);
    }
}

void CalendarWidget::onDayClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QDate date = btn->property("date").toDate();
    if (!date.isValid()) return;

    addInterval dlg(date,this);
    dlg.exec();

    rebuildCalendarGrid();
}

void CalendarWidget::on_pushButtonDeleteData_clicked()
{
    int year = ui->spinBoxYear->value();
    int month = ui->comboBoxMonth->currentIndex() + 1;
    int user_id = Session::instance().userId();
    if(user_id == -1) return;

    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle("Удаление данных");
    msg.setText("Выберите промежуток за который вы хотите удалить данные");

    QPushButton *btnDeleteAll = msg.addButton("Удалить всё", QMessageBox::DestructiveRole);
    QPushButton *btnDeleteYear = msg.addButton("Удалить за год", QMessageBox::ActionRole);
    QPushButton *btnDeleteMonth = msg.addButton("Удалить за месяц", QMessageBox::ActionRole);
    msg.addButton("Отмена",QMessageBox::RejectRole);

    msg.exec();

    if (msg.clickedButton() == btnDeleteAll){

        QMessageBox confirm(this);
        confirm.setIcon(QMessageBox::Warning);
        confirm.setWindowTitle("Подтверждение");
        confirm.setText(QString("Удалить данные за все время?"));
        confirm.setInformativeText("Это действие нельзя отменить.");
        QPushButton *btnDelete = confirm.addButton("Удалить", QMessageBox::DestructiveRole);
        confirm.addButton("Отмена", QMessageBox::RejectRole);

        confirm.exec();
        if(confirm.clickedButton() != btnDelete) return;

        QSqlQuery query(Database::instance().db);
        query.prepare("DELETE FROM work_intervals WHERE user_id = ?");
        query.addBindValue(user_id);

        if(query.exec()){
            rebuildCalendarGrid();
        } else {
            qDebug() << "Ошибка удаления данных: " << query.lastError().text();
        }
    }
    else if(msg.clickedButton() == btnDeleteYear){
        QDate first(year, 1, 1);
        QDate last(year, 12, first.daysInMonth());

        QMessageBox confirm(this);
        confirm.setIcon(QMessageBox::Warning);
        confirm.setWindowTitle("Подтверждение");
        confirm.setText(QString("Удалить данные за %1 год?").arg(year));
        confirm.setInformativeText("Это действие нельзя отменить.");
        QPushButton *btnDelete = confirm.addButton("Удалить", QMessageBox::DestructiveRole);
        confirm.addButton("Отмена", QMessageBox::RejectRole);

        confirm.exec();
        if(confirm.clickedButton() != btnDelete) return;

        QSqlQuery query(Database::instance().db);
        query.prepare("DELETE FROM work_intervals "
                      "WHERE user_id = ? AND work_date BETWEEN ? AND ?");
        query.addBindValue(user_id);
        query.addBindValue(first);
        query.addBindValue(last);

        if (query.exec()) rebuildCalendarGrid();
        else qDebug() << "Ошибка удаления данных:" << query.lastError().text();
    }
    else if(msg.clickedButton() == btnDeleteMonth){
        QDate first(year, month, 1);
        QDate last(year, month, first.daysInMonth());

        QMessageBox confirm(this);
        confirm.setIcon(QMessageBox::Warning);
        confirm.setWindowTitle("Подтверждение");
        confirm.setText(QString("Удалить данные за %1.%2?").arg(month, 2, 10, QChar('0')).arg(year));
        confirm.setInformativeText("Это действие нельзя отменить.");
        QPushButton *btnDelete = confirm.addButton("Удалить", QMessageBox::DestructiveRole);
        confirm.addButton("Отмена", QMessageBox::RejectRole);

        confirm.exec();
        if(confirm.clickedButton() != btnDelete) return;

        QSqlQuery query(Database::instance().db);
        query.prepare("DELETE FROM work_intervals "
                      "WHERE user_id = ? AND work_date BETWEEN ? AND ?");
        query.addBindValue(user_id);
        query.addBindValue(first);
        query.addBindValue(last);

        if (query.exec()) rebuildCalendarGrid();
        else qDebug() << "Ошибка удаления данных:" << query.lastError().text();
    }
    else{
        return;
    }
}

void CalendarWidget::on_pushButtonToStat_clicked()
{
    emit openStatistic();
}


static QString csvEscape(QString s)
{
    s.replace("\"", "\"\"");
    bool needQuotes = s.contains(',') || s.contains(';') || s.contains('\n') || s.contains('\r') || s.contains('"');
    return needQuotes ? QString("\"%1\"").arg(s) : s;
}

void CalendarWidget::on_pushButtonSaveInFile_clicked()
{
    int userId = Session::instance().userId();
    if(userId == -1) return;

    QString path = QFileDialog::getSaveFileName(this,
                        "Сохранить в CSV",
                        "work_intervals.csv"
                        ,"CSV (*.csv)");
    if(path.isEmpty()) return;

    QFile file(path);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);

    out.setEncoding(QStringConverter::Utf8);
    out << QChar(0xFEFF);

    out << ("Дата;Начало раб. времени;Конец раб. времени;Отработанное время;Заметка\n");

    QSqlQuery q(Database::instance().db);

    q.prepare("SELECT work_date, start_time, end_time, note, "
              "COALESCE((EXTRACT(EPOCH FROM (end_time - start_time))/60),0)::int AS minutes "
              "FROM work_intervals WHERE user_id = ? "
              "ORDER BY work_date, start_time");
    q.addBindValue(userId);

    if(!q.exec()){
        QMessageBox::critical(this, "Ошибка", "Ошибка запроса: " + q.lastError().text());
        return;
    }

    while (q.next())
    {
        QString date  = q.value("work_date").toDate().toString("yyyy-MM-dd");
        QString start = q.value("start_time").toTime().toString("HH:mm");
        QString end   = q.value("end_time").toTime().toString("HH:mm");

        int minutes = q.value("minutes").toInt();
        QString time = TimeUtils::formatWorkedRu(minutes);
        QString note = q.value("note").toString();

        out << date    << ";"
            << start   << ";"
            << end     << ";"
            << time    << ";"
            << csvEscape(note)
            << "\n";
    }

    file.close();
    QMessageBox::information(this, "Готово", "CSV сохранён:\n " + path);
}

