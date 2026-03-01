#ifndef ADDINTERVAL_H
#define ADDINTERVAL_H

#include <QDialog>
#include <QDate>

namespace Ui {
class addInterval;
}

class addInterval : public QDialog
{
    Q_OBJECT

public:
    explicit addInterval(const QDate &date, QWidget *parent = nullptr);
    ~addInterval();

private slots:
    void on_pushButtonAdd_clicked();
    bool checkTime(int userId, const QDate &date, const QTime &start, const QTime &end, int excludeId);

    void on_pushButtonDelete_clicked();

    void on_tableIntervals_itemSelectionChanged();

    void on_pushButtonEdit_clicked();

private:
    Ui::addInterval *ui;
    void loadIntervals();
    QDate m_date;
    int m_selectedId = -1;
};

#endif // ADDINTERVAL_H
