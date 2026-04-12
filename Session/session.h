#ifndef SESSION_H
#define SESSION_H

#include <QString>

class Session
{
public:
    static Session& instance();

    void setUser(int id, const QString &login, const QString &name, const QString &mail, const QString &photo, int moneyForHour, int profession_id);
    void aboutUser() const;
    void clear();

    int userId() const;
    int moneyForHour() const;
    QString login() const;
    QString name() const;
    QString mail() const;
    QString photo() const;
    int profession_id() const;
    void setMoneyForHour(int value);
    void setProfessionId(int value);
    bool isLoggedIn() const;

private:
    Session();
    int m_userId;
    int m_moneyForHour;
    QString m_login;
    QString m_name;
    QString m_mail;
    QString m_photo;
    int m_profession_id;
};

#endif // SESSION_H
