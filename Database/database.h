#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>

class Database
{
public:
    static Database& instance();
    bool connect();

    QSqlDatabase db;

private:
    Database();
};

#endif
