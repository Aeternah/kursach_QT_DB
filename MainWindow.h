#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include "DatabaseManager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectToDatabase();
    void onDisconnectFromDatabase();
    void onExecuteQuery();
    void onConnectionSelected(int index);
    void onTableSelected(int index);
    void onExportToCSV();
    void onBrowseClicked();
    void onOpenQueryBuilder();  // Новый слот
    void saveToHistory(const QString &query);  // Для истории запросов
    

private:
    void updateConnectionsList();
    void updateTablesList(const QString &connectionName);
    void updateQueryResults(QSqlQuery query);
    void showError(const QString &message);
    void loadHistory();
    void saveHistory();
    QList<QString> m_queryHistory;
    

    Ui::MainWindow *ui;
    DatabaseManager *dbManager;
};

#endif // MAINWINDOW_H