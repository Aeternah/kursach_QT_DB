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
    void onAddConditionClicked();
    void onGenerateQuery();
    void onCopyClicked();


private:
    void refreshTables();
    
    Ui::QueryBuilderDialog *ui;
    DatabaseManager *m_dbManager;
    QString m_connectionName;
    QList<QString> m_conditions; // Условия WHERE
};

#endif
