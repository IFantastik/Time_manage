#include <QApplication>
#include "Dialogs/loginwindow.h"
#include "Windows/mainwindow.h"
#include "Database/database.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(!Database::instance().connect())
        return -1;

    loginwindow login;
    if (login.exec() != QDialog::Accepted) return 0;

    MainWindow mainWin;
    mainWin.show();

    return a.exec();
}
