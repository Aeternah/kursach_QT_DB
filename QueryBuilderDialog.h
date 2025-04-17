#ifndef QUERYBUILDERDIALOG_H
#define QUERYBUILDERDIALOG_H

#include <QDialog>
#include <QStringList>
#include "DatabaseManager.h"

namespace Ui {
class QueryBuilderDialog;
}

class QueryBuilderDialog : public QDialog {
    Q_OBJECT
public:
    explicit QueryBuilderDialog(DatabaseManager *dbManager, const QString &connectionName, QWidget *parent = nullptr);
    ~QueryBuilderDialog();
    QString generatedQuery() const;

private slots:
    void onTableSelected(int index);
    void onAddJoinClicked();
    void onAddConditionClicked();
    void onGenerateQuery();

private:
    void refreshTables();
    void refreshColumns();
    Ui::QueryBuilderDialog *ui;
    DatabaseManager *m_dbManager;
    QString m_connectionName;
    QList<QPair<QString, QString>> m_joins; // Пары (таблица, условие JOIN)
    QList<QString> m_conditions; // Условия WHERE
};

#endif