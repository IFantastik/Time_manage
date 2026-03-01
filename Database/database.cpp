#include "database.h"
#include <QSqlError>
#include <QDebug>

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
    db = QSqlDatabase::addDatabase("QPSQL");

    db.setHostName("localhost");
    db.setDatabaseName("timeman");
    db.setUserName("postgres");
    db.setPassword("1252");

    if(!db.open())
    {
        qDebug() << db.lastError();
        return false;
    }

    return true;
}
