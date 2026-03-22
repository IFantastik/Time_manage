#include "statistic.h"
#include "ui_statistic.h"
#include "Session/session.h"
#include "Database/database.h"
#include "qcustomplot.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>

#include <QApplication>
#include <QPalette>
#include <QEvent>

Statistic::Statistic(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Statistic)
{
    ui->setupUi(this);

    setupUiDefaults();
    setupPlot();

    connect(ui->dateEditStart, &QDateEdit::dateChanged,
            this, &Statistic::onRangeChanged);
    connect(ui->dateEditEnd, &QDateEdit::dateChanged,
            this, &Statistic::onRangeChanged);

    reloadAndPlot();
}

Statistic::~Statistic()
{
    delete ui;
}

void Statistic::setupUiDefaults()
{
    const QDate today = QDate::currentDate();

    ui->dateEditStart->setCalendarPopup(true);
    ui->dateEditEnd->setCalendarPopup(true);

    ui->dateEditEnd->setDate(today);
    ui->dateEditStart->setDate(today.addDays(-6));
}

void Statistic::setupPlot()
{

    m_bars = new QCPBars(ui->widget->xAxis, ui->widget->yAxis);
    m_bars->setPen(QPen(Qt::transparent));
    m_bars->setWidth(0.8);

    ui->widget->xAxis->setLabel("Период");
    ui->widget->yAxis->setLabel("Минуты");

    ui->widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->widget->xAxis->setTickLabelRotation(-45);

    applySystemTheme();
    ui->widget->replot();
}

void Statistic::applySystemTheme()
{
    const QPalette pal = QApplication::palette();

    const QColor windowBg  = pal.color(QPalette::Window);
    const QColor textColor = pal.color(QPalette::WindowText);

    QColor gridColor = textColor.lighter(140);
    gridColor.setAlpha(40);

    ui->widget->setBackground(windowBg);
    ui->widget->axisRect()->setBackground(windowBg);

    ui->widget->xAxis->setBasePen(QPen(textColor));
    ui->widget->yAxis->setBasePen(QPen(textColor));

    ui->widget->xAxis->setTickPen(QPen(textColor));
    ui->widget->yAxis->setTickPen(QPen(textColor));
    ui->widget->xAxis->setSubTickPen(QPen(textColor));
    ui->widget->yAxis->setSubTickPen(QPen(textColor));

    ui->widget->xAxis->setTickLabelColor(textColor);
    ui->widget->yAxis->setTickLabelColor(textColor);
    ui->widget->xAxis->setLabelColor(textColor);
    ui->widget->yAxis->setLabelColor(textColor);

    ui->widget->xAxis->grid()->setPen(QPen(gridColor, 1, Qt::DotLine));
    ui->widget->yAxis->grid()->setPen(QPen(gridColor, 1, Qt::DotLine));
    ui->widget->xAxis->grid()->setSubGridVisible(false);
    ui->widget->yAxis->grid()->setSubGridVisible(false);

    const QColor accent = pal.color(QPalette::Highlight);
    if (m_bars) m_bars->setBrush(QBrush(accent));
}

void Statistic::changeEvent(QEvent *event)
{
    if (event && event->type() == QEvent::ApplicationPaletteChange) {
        applySystemTheme();
        ui->widget->replot();
    }
    QWidget::changeEvent(event);
}

void Statistic::onRangeChanged()
{
    QDate start = ui->dateEditStart->date();
    QDate end   = ui->dateEditEnd->date();

    if (start > end) {
        end = start;
        ui->dateEditEnd->setDate(end);
    }

    const int days = start.daysTo(end) + 1;
    if (days > kMaxDaysRange) {
        end = start.addDays(kMaxDaysRange - 1);
        ui->dateEditEnd->setDate(end);
    }

    reloadAndPlot();
}

void Statistic::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    reloadAndPlot();
}

void Statistic::reloadAndPlot()
{
    if (!m_bars) return;

    const int userId = Session::instance().userId();
    if (userId == -1) {
        m_bars->setData(QVector<double>(), QVector<double>());
        ui->widget->replot();
        return;
    }

    const QDate from = ui->dateEditStart->date();
    const QDate to = ui->dateEditEnd->date();

    const GroupMode mode = chooseGroupMode(from, to);

    QVector<QDate> buckets;
    QVector<double> minutes;

    if (!loadMinutesGrouped(from, to, buckets, minutes, mode)) {
        m_bars->setData(QVector<double>(), QVector<double>());
        ui->widget->replot();
        return;
    }

    plotBars(buckets, minutes, mode);
}

Statistic::GroupMode Statistic::chooseGroupMode(const QDate &from, const QDate &to) const
{
    const int days = from.daysTo(to) + 1;

    if (days <= 31){
        ui->widget->xAxis->setLabel("Период по дням (один столбец - один день)");
        return GroupMode::Day;
    }
    if (days <= 180){
        ui->widget->xAxis->setLabel("Период по неделям (один столбец - одна неделя)");
        return GroupMode::Week;
    }
    ui->widget->xAxis->setLabel("Период по месяцам (один столбец - один месяц)");
    return GroupMode::Month;
}

QDate Statistic::weekStart(const QDate &d)
{
    return d.addDays(-(d.dayOfWeek() - 1));
}

QDate Statistic::monthStart(const QDate &d)
{
    return QDate(d.year(), d.month(), 1);
}

bool Statistic::loadMinutesGrouped(const QDate &from, const QDate &to,
                                   QVector<QDate> &bucketStarts,
                                   QVector<double> &minutes,
                                   GroupMode mode)
{
    bucketStarts.clear();
    minutes.clear();

    const int userId = Session::instance().userId();
    if (userId == -1) return false;

    QString bucketExpr;
    switch (mode) {
    case GroupMode::Day:
        bucketExpr = "work_date";
        break;
    case GroupMode::Week:

        bucketExpr = "date_trunc('week', work_date)::date";
        break;
    case GroupMode::Month:
        bucketExpr = "date_trunc('month', work_date)::date";
        break;
    }

    QSqlQuery q(Database::instance().db);
    q.prepare(
        "SELECT " + bucketExpr + " AS bucket, "
                                 "       COALESCE(SUM(EXTRACT(EPOCH FROM (end_time - start_time)) / 60), 0)::int AS minutes "
                                 "FROM work_intervals "
                                 "WHERE user_id = ? AND is_active = ? AND work_date BETWEEN ? AND ? "
                                 "GROUP BY bucket "
                                 "ORDER BY bucket"
        );

    q.addBindValue(userId);
    q.addBindValue(true);
    q.addBindValue(from.toString(Qt::ISODate));
    q.addBindValue(to.toString(Qt::ISODate));

    if (!q.exec()) {
        qDebug() << "Ошибка загрузки данных для статистики:" << q.lastError().text();
        return false;
    }

    QMap<QDate, int> map;
    while (q.next()) {
        const QDate bucket = q.value(0).toDate();
        const int mins = q.value(1).toInt();
        map[bucket] = mins;
    }

    if (mode == GroupMode::Day) {
        for (QDate d = from; d <= to; d = d.addDays(1)) {
            bucketStarts.push_back(d);
            minutes.push_back(map.value(d, 0));
        }
        return true;
    }

    if (mode == GroupMode::Week) {
        QDate start = weekStart(from);
        QDate end   = weekStart(to);

        for (QDate d = start; d <= end; d = d.addDays(7)) {
            bucketStarts.push_back(d);
            minutes.push_back(map.value(d, 0));
        }
        return true;
    }

    {
        QDate start = monthStart(from);
        QDate end   = monthStart(to);

        for (QDate d = start; d <= end; d = d.addMonths(1)) {
            bucketStarts.push_back(d);
            minutes.push_back(map.value(d, 0));
        }
        return true;
    }
}

void Statistic::plotBars(const QVector<QDate> &buckets,
                         const QVector<double> &mins,
                         GroupMode mode)
{
    QVector<double> x;
    x.reserve(buckets.size());

    auto ticker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText);

    for (int i = 0; i < buckets.size(); ++i) {
        const double xi = i + 1;
        x.push_back(xi);

        QString label;
        if (mode == GroupMode::Day) {
            label = buckets[i].toString("dd.MM");
        } else if (mode == GroupMode::Week) {
            label = buckets[i].toString("dd.MM") + " - " + buckets[i].addDays(6).toString("dd.MM");
        } else {
            label = buckets[i].toString("MM.yyyy");
        }

        ticker->addTick(xi, label);
    }

    ui->widget->xAxis->setTicker(ticker);

    m_bars->setData(x, mins);

    ui->widget->xAxis->setRange(0.0, x.size() + 1.0);

    double maxY = 0.0;
    for (double v : mins) maxY = qMax(maxY, v);
    const double top = (maxY <= 0.0) ? 10.0 : (maxY * 1.2 + 10.0);
    ui->widget->yAxis->setRange(0.0, top);

    ui->widget->replot();
}

void Statistic::on_pushButtonToCalendar_clicked()
{
    emit openCalendar();
}
