#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QFileDialog>
#include <QTextStream>
#include <QSqlError>  // Для QSqlError
#include <QSqlRecord>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dbManager(new DatabaseManager(this))
{
    ui->setupUi(this);

    // Connect signals and slots
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectToDatabase);
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectFromDatabase);
    connect(ui->btnExecute, &QPushButton::clicked, this, &MainWindow::onExecuteQuery);
    connect(ui->btnExport, &QPushButton::clicked, this, &MainWindow::onExportToCSV);
    connect(ui->cbConnections, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onConnectionSelected);
    connect(ui->cbTables, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onTableSelected);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectToDatabase()
{
    DatabaseManager::DatabaseType dbType = ui->rbSQLite->isChecked() ? 
                                         DatabaseManager::SQLite : 
                                         DatabaseManager::PostgreSQL;
    
    QString connectionName = ui->leConnectionName->text().trimmed();
    if (connectionName.isEmpty()) {
        showError("Please enter connection name");
        return;
    }
    
    bool success = false;
    if (dbType == DatabaseManager::SQLite) {
        QString dbPath = ui->leSQLitePath->text().trimmed();
        success = dbManager->connectToDatabase(dbType, connectionName, dbPath);
    } else {
        QString dbName = ui->lePGDatabase->text().trimmed();
        QString host = ui->lePGHost->text().trimmed();
        QString user = ui->lePGUser->text().trimmed();
        QString password = ui->lePGPassword->text().trimmed();
        int port = ui->sbPGPort->value();
        
        success = dbManager->connectToDatabase(dbType, connectionName, dbName, 
                                             host, user, password, port);
    }
    
    if (success) {
        updateConnectionsList();
        QMessageBox::information(this, "Success", "Connected successfully");
    } else {
        showError(dbManager->lastError());
    }
}

void MainWindow::onDisconnectFromDatabase()
{
    QString connectionName = ui->cbConnections->currentText();
    if (connectionName.isEmpty()) return;
    
    dbManager->disconnectFromDatabase(connectionName);
    updateConnectionsList();
}

void MainWindow::onExecuteQuery()
{
    QString connectionName = ui->cbConnections->currentText();
    if (connectionName.isEmpty()) {
        showError("No connection selected");
        return;
    }
    
    QString queryText = ui->pteQuery->toPlainText().trimmed();
    if (queryText.isEmpty()) {
        showError("Query is empty");
        return;
    }
    
    QSqlQuery result = dbManager->executeQuery(queryText, connectionName);
    updateQueryResults(std::move(result));
}

void MainWindow::onConnectionSelected(int index)
{
    if (index < 0) return;
    QString connectionName = ui->cbConnections->currentText();
    updateTablesList(connectionName);
}

void MainWindow::onTableSelected(int index)
{
    if (index < 0) return;
    
    QString connectionName = ui->cbConnections->currentText();
    QString tableName = ui->cbTables->currentText();
    
    if (!connectionName.isEmpty() && !tableName.isEmpty()) {
        QString query = QString("SELECT * FROM %1 LIMIT 100").arg(tableName);
        QSqlQuery result = dbManager->executeQuery(query, connectionName);
        updateQueryResults(std::move(result));
    }
}

void MainWindow::onExportToCSV()
{
    QAbstractItemModel *model = ui->tvResults->model();
    if (!model) return;
    
    QString fileName = QFileDialog::getSaveFileName(this, "Export to CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        
        // Write headers
        for (int col = 0; col < model->columnCount(); ++col) {
            if (col > 0) out << ",";
            out << "\"" << model->headerData(col, Qt::Horizontal).toString().replace("\"", "\"\"") << "\"";
        }
        out << "\n";
        
        // Write data
        for (int row = 0; row < model->rowCount(); ++row) {
            for (int col = 0; col < model->columnCount(); ++col) {
                if (col > 0) out << ",";
                out << "\"" << model->data(model->index(row, col)).toString().replace("\"", "\"\"") << "\"";
            }
            out << "\n";
        }
        
        file.close();
        QMessageBox::information(this, "Success", "Data exported successfully");
    } else {
        showError("Failed to save file");
    }
}

void MainWindow::updateConnectionsList()
{
    ui->cbConnections->clear();
    ui->cbConnections->addItems(dbManager->activeConnections());
}

void MainWindow::updateTablesList(const QString &connectionName)
{
    ui->cbTables->clear();
    ui->cbTables->addItems(dbManager->getTables(connectionName));
}

void MainWindow::updateQueryResults(QSqlQuery query)
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(std::move(query));
    ui->tvResults->setModel(model);
    
    if (model->lastError().isValid()) {
        showError(model->lastError().text());
    }
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, "Error", message);
}