#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Проверка доступности драйверов БД
    qDebug() << "Available database drivers:";
    for(const QString &driver : QSqlDatabase::drivers())
        qDebug() << "  " << driver;

    MainWindow w;
    w.show();
    
    return a.exec();
}