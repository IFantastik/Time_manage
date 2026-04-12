#include "session.h"
#include <QString>
#include <QDebug>

Session::Session() {
    m_userId = -1;
    m_moneyForHour = 0;
    m_profession_id = -1;
}

Session& Session::instance()
{
    static Session s;
    return s;
}

void Session::setUser(int id, const QString &login, const QString &name, const QString &mail, const QString &photo, int moneyForHour, int profession_id)
{
    m_userId = id;
    m_login = login;
    m_name = name;
    m_mail = mail;
    m_photo = photo;
    m_moneyForHour = moneyForHour;
    m_profession_id = profession_id;
}

void Session::clear()
{
    m_userId = -1;
    m_login.clear();
    m_name.clear();
    m_mail.clear();
    m_photo.clear();
    m_moneyForHour = 0;
    m_profession_id = -1;
}

void Session::aboutUser() const{
    qDebug() << "User ID:" << m_userId;
    qDebug() << "Login:" << m_login;
    qDebug() << "Name:" << m_name;
    qDebug() << "Mail:" << m_mail;
    qDebug() << "Photo:" << m_photo;
    qDebug() << "Money for hour:" << m_moneyForHour;
    qDebug() << "Profession_id:" << m_profession_id;
}

int Session::userId() const
{
    return m_userId;
}

int Session::moneyForHour() const
{
    return m_moneyForHour;
}

void Session::setMoneyForHour(int value){
    m_moneyForHour = value;
}

void Session::setProfessionId(int value){
    m_profession_id = value;
}

QString Session::login() const
{
    return m_login;
}

QString Session::name() const
{
    return m_name;
}

QString Session::mail() const
{
    return m_mail;
}

QString Session::photo() const
{
    return m_photo;
}

bool Session::isLoggedIn() const
{
    return m_userId != -1;
}

int Session::profession_id() const
{
    return m_profession_id;
}
