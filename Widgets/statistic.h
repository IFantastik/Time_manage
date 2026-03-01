#ifndef STATISTIC_H
#define STATISTIC_H

#include <QWidget>
#include <QDate>
#include <QVector>

class QCPBars;
class QEvent;

namespace Ui {
class Statistic;
}

class Statistic : public QWidget
{
    Q_OBJECT

public:
    explicit Statistic(QWidget *parent = nullptr);
    ~Statistic() override;

signals:
    void openCalendar();

private slots:
    void on_pushButtonToCalendar_clicked();
    void onRangeChanged();

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    enum class GroupMode { Day, Week, Month };

    static constexpr int kMaxDaysRange = 1000;

    void setupUiDefaults();
    void setupPlot();

    void applySystemTheme();

    void reloadAndPlot();

    GroupMode chooseGroupMode(const QDate &from, const QDate &to) const;

    static QDate weekStart(const QDate &d);
    static QDate monthStart(const QDate &d);

    bool loadMinutesGrouped(const QDate &from, const QDate &to,
                            QVector<QDate> &bucketStarts,
                            QVector<double> &minutes,
                            GroupMode mode);

    void plotBars(const QVector<QDate> &buckets,
                  const QVector<double> &mins,
                  GroupMode mode);

private:
    Ui::Statistic *ui = nullptr;
    QCPBars *m_bars = nullptr;
};

#endif // STATISTIC_H
