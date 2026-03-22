#include "database.h"
#include <QSqlError>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>

Database::Database()
{
}

Database& Database::instance()
{
    static Database instance;
    return instance;
}

bool Database::connect()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    QString host = settings.value("db/host").toString();
    QString name = settings.value("db/name").toString();
    QString user = settings.value("db/user").toString();
    QString pass = settings.value("db/password").toString();

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(host);
    db.setDatabaseName(name);
    db.setUserName(user);
    db.setPassword(pass);

    if(!db.open())
    {
        QMessageBox::critical(nullptr,
                              "Ошибка подключения базы данных: ",
                              db.lastError().text());
        return false;
    }

    return true;
}
