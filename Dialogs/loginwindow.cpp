#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "QDebug"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QString>
#include "Database/database.h"
#include "Session/session.h"

loginwindow::loginwindow(QDialog *parent)
    : QDialog(parent)
    , ui(new Ui::loginwindow)
{
    ui->setupUi(this);

    setWindowFlag(Qt::Window, true);
    setWindowTitle("Вход");
}

loginwindow::~loginwindow()
{
    delete ui;
}

void loginwindow::on_pushButtonToReg_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void loginwindow::on_pushButtonToJoin_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

QString hashPassword(const QString &password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

void loginwindow::checkLogin(QString login, QString password){
    if (!Database::instance().db.isOpen()){
        qDebug() << "База данных не открыта!";
        return;
    }
    QSqlQuery query(Database::instance().db);
    query.prepare("SELECT user_id, password, name, mail, photo, money_for_hour, profession_id FROM users WHERE login = ?");
    query.addBindValue(login);

    if(!query.exec()){
        ui->labelJoin_status->setText("Ошибка запроса: " + query.lastError().text());
        qDebug() << "Ошибка запроса: " + query.lastError().text();
        return;
    }

    if(query.next()){
        int user_id = query.value(0).toInt();
        QString storedHash = query.value(1).toString();
        QString enteredHash = hashPassword(password);
        QString name = query.value(2).toString();
        QString mail = query.value(3).toString();
        QString photo = query.value(4).toString();
        int moneyForHour = query.value(5).toInt();
        int profession_id = query.value(6).toInt();
        if(storedHash == enteredHash){
            Session::instance().setUser(user_id, login, name, mail, photo, moneyForHour, profession_id);
            accept();
        }
        else{
            ui->labelJoin_status->setText("Неверный пароль");
        }
    }
    else{
        ui->labelJoin_status->setText("Пользователь не найден");
    }
}

void loginwindow::addUser(QString login, QString password, QString repeatpas, QString mail, QString name){
    if (!Database::instance().db.isOpen()) {
        qDebug() << "База не открыта!";
        return;
    }
    if(login.isEmpty() || password.isEmpty() || repeatpas.isEmpty() || mail.isEmpty() || name.isEmpty()){
        ui->labelReg_status->setText("Не все поля заполнены");
        return;
    }
    if (repeatpas!=password){
        ui->labelReg_status->setText("Пароли не совпадают");
        return;
    }
    if (password.length() < 8){
        ui->labelReg_status->setText("Пароль должен содержать хотя бы 8 символов");
        return;
    }
    if (login.length() < 3){
        ui->labelReg_status->setText("Логин должен содержать хотя бы 3 символа");
        return;
    }
    if(!mail.contains("@")){
        ui->labelReg_status->setText("Email введен неверно");
        return;
    }

    QString hashedpassword = hashPassword(password);
    QSqlQuery query(Database::instance().db);

    query.prepare("INSERT INTO users (login, password, name, mail, photo) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(login);
    query.addBindValue(hashedpassword);
    query.addBindValue(name);
    query.addBindValue(mail);
    query.addBindValue(":/images/avatar/image/default.png");

    if(!query.exec()){
        ui->labelReg_status->setText("Ошибка регистрации: " + query.lastError().text());
        qDebug() << query.lastError();
        return;
    }

    ui->labelJoin_status->setStyleSheet("color: green;");
    ui->labelJoin_status->setText("Регистрация прошла успешно!");
    ui->stackedWidget->setCurrentIndex(0);
}

void loginwindow::on_pushButtonReg_clicked()
{
    QString login = ui->lineEditReg_log->text();
    QString password = ui->lineEditReg_pas->text();
    QString repeatpas = ui->lineEditReg_repeatpas->text();
    QString mail = ui->lineEditReg_mail->text();
    QString name = ui->lineEditReg_name->text();
    addUser(login,password,repeatpas,mail,name);
}

void loginwindow::on_pushButtonJoin_clicked()
{
    ui->labelJoin_status->setStyleSheet("color: red;");
    QString login = ui->lineEditJoin_log->text();
    QString password = ui->lineEditJoin_pas->text();

    checkLogin(login, password);
}

void loginwindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    ui->lineEditJoin_log->clear();
    ui->lineEditJoin_pas->clear();
    ui->labelJoin_status->clear();
    ui->labelReg_status->clear();
}
