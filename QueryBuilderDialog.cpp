#include "QueryBuilderDialog.h"
#include "ui_QueryBuilderDialog.h"

QueryBuilderDialog::QueryBuilderDialog(DatabaseManager *dbManager, const QString &connectionName, QWidget *parent)
    : QDialog(parent), ui(new Ui::QueryBuilderDialog), m_dbManager(dbManager), m_connectionName(connectionName) {
    ui->setupUi(this);
    
    // Настройка интерфейса
    ui->comboJoinType->addItems({"INNER JOIN", "LEFT JOIN", "RIGHT JOIN"});
    refreshTables();
    
    // Соединения сигналов
    connect(ui->comboMainTable, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QueryBuilderDialog::onTableSelected);
    connect(ui->btnAddJoin, &QPushButton::clicked,
            this, &QueryBuilderDialog::onAddJoinClicked);
    connect(ui->btnAddCondition, &QPushButton::clicked,
            this, &QueryBuilderDialog::onAddConditionClicked);
    connect(ui->btnGenerate, &QPushButton::clicked,
            this, &QueryBuilderDialog::onGenerateQuery);
}

void QueryBuilderDialog::refreshTables() {
    ui->comboMainTable->clear();
    ui->comboJoinTable->clear();
    QStringList tables = m_dbManager->getTables(m_connectionName);
    ui->comboMainTable->addItems(tables);
    ui->comboJoinTable->addItems(tables);
}

void QueryBuilderDialog::onTableSelected(int index) {
    if (index < 0) return;
    QString table = ui->comboMainTable->currentText();
    ui->listColumns->clear();
    for (const QString &column : m_dbManager->getTableColumns(table, m_connectionName)) {
        QListWidgetItem *item = new QListWidgetItem(column, ui->listColumns);
        item->setCheckState(Qt::Unchecked);
    }
}

void QueryBuilderDialog::onAddJoinClicked() {
    QString joinTable = ui->comboJoinTable->currentText();
    QString condition = ui->editJoinCondition->text();
    if (!joinTable.isEmpty() && !condition.isEmpty()) {
        m_joins.append(qMakePair(joinTable, condition));
        ui->listJoins->addItem(QString("%1 ON %2").arg(joinTable).arg(condition));
    }
}

void QueryBuilderDialog::onAddConditionClicked() {
    QString condition = ui->editCondition->text();
    if (!condition.isEmpty()) {
        m_conditions.append(condition);
        ui->listConditions->addItem(condition);
    }
}

void QueryBuilderDialog::onGenerateQuery() {
    QString mainTable = ui->comboMainTable->currentText();
    QStringList columns;
    for (int i = 0; i < ui->listColumns->count(); ++i) {
        if (ui->listColumns->item(i)->checkState() == Qt::Checked) {
            columns << ui->listColumns->item(i)->text();
        }
    }
    
    // Формируем SQL
    QString query = "SELECT ";
    query += columns.isEmpty() ? "*" : columns.join(", ");
    query += " FROM " + mainTable;
    
    // Добавляем JOIN
    for (const auto &join : m_joins) {
        query += " " + join.first + " ON " + join.second;
    }
    
    // Добавляем WHERE
    if (!m_conditions.isEmpty()) {
        query += " WHERE " + m_conditions.join(" AND ");
    }
    
    ui->textQuery->setPlainText(query);
}

// === ДОБАВЛЕННЫЕ МЕТОДЫ ===

QueryBuilderDialog::~QueryBuilderDialog() {
    delete ui;
}

QString QueryBuilderDialog::generatedQuery() const {
    return ui->textQuery->toPlainText();
}
