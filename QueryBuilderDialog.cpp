#include "QueryBuilderDialog.h"
#include <QClipboard>
#include "ui_QueryBuilderDialog.h"

QueryBuilderDialog::QueryBuilderDialog(DatabaseManager *dbManager, const QString &connectionName, QWidget *parent)
    : QDialog(parent), ui(new Ui::QueryBuilderDialog), m_dbManager(dbManager), m_connectionName(connectionName) {
    ui->setupUi(this);

    refreshTables();

    // Соединения сигналов
    connect(ui->comboMainTable, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QueryBuilderDialog::onTableSelected);
    connect(ui->btnAddCondition, &QPushButton::clicked,
            this, &QueryBuilderDialog::onAddConditionClicked);
    connect(ui->btnGenerate, &QPushButton::clicked,
            this, &QueryBuilderDialog::onGenerateQuery);
    connect(ui->btnCopy, &QPushButton::clicked,
            this, &QueryBuilderDialog::onCopyClicked);
        
}

void QueryBuilderDialog::refreshTables() {
    ui->comboMainTable->clear();
    QStringList tables = m_dbManager->getTables(m_connectionName);
    ui->comboMainTable->addItems(tables);
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

void QueryBuilderDialog::onAddConditionClicked() {
    QString condition = ui->editCondition->text();
    if (!condition.isEmpty()) {
        m_conditions.append(condition);
        ui->listConditions->addItem(condition);
    }
}
void QueryBuilderDialog::onCopyClicked() {
    QString query = ui->textQuery->toPlainText();
    if (!query.isEmpty()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(query);
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


    QString query = "SELECT ";
    query += columns.isEmpty() ? "*" : columns.join(", ");
    query += " FROM " + mainTable;

    if (!m_conditions.isEmpty()) {
        query += " WHERE " + m_conditions.join(" AND ");
    }

    ui->textQuery->setPlainText(query);
}

QueryBuilderDialog::~QueryBuilderDialog() {
    delete ui;
}

QString QueryBuilderDialog::generatedQuery() const {
    return ui->textQuery->toPlainText();
}
