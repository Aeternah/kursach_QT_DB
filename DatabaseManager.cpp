#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlError>
#include <QFileInfo>
#include <QSqlRecord> // Для QSqlRecord
#include <QSqlQueryModel>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    auto connections = m_connections.keys();
    for (auto &connectionName : connections) {
        disconnectFromDatabase(connectionName);
    }
}

bool DatabaseManager::connectToDatabase(DatabaseType type, const QString &connectionName,
                                     const QString &databaseName,
                                     const QString &host,
                                     const QString &user,
                                     const QString &password,
                                     int port)
{
    if (m_connections.contains(connectionName)) {
        m_lastError = "Connection with this name already exists";
        return false;
    }

    if (type == SQLite) {
        QFileInfo dbFile(databaseName);
        if (!dbFile.exists()) {
            m_lastError = "Database file does not exist";
            return false;
        }
    }

    QSqlDatabase db;
    if (type == SQLite) {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(databaseName);
    } 
    else if (type == PostgreSQL) {
        db = QSqlDatabase::addDatabase("QPSQL", connectionName);
        db.setDatabaseName(databaseName);
        db.setHostName(host);
        db.setUserName(user);
        db.setPassword(password);
        if (port > 0) db.setPort(port);
    }

    if (!db.open()) {
        m_lastError = db.lastError().text();
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }

    m_connections[connectionName] = db;
    m_tablesCache.remove(connectionName); // Clear cache for this connection
    return true;
}

void DatabaseManager::disconnectFromDatabase(const QString &connectionName)
{
    if (m_connections.contains(connectionName)) {
        m_connections[connectionName].close();
        m_connections.remove(connectionName);
        QSqlDatabase::removeDatabase(connectionName);
        m_tablesCache.remove(connectionName);
    }
}

QSqlQuery DatabaseManager::executeQuery(const QString &query, const QString &connectionName)
{
    if (!m_connections.contains(connectionName)) {
        m_lastError = "Connection not found: " + connectionName;
        return QSqlQuery();
    }

    QSqlDatabase db = m_connections[connectionName];
    if (!db.isOpen()) {
        m_lastError = "Database not open: " + connectionName;
        return QSqlQuery();
    }

    QSqlQuery qry(db);
    if (!qry.prepare(query)) {
        m_lastError = qry.lastError().text();
        return QSqlQuery();
    }

    if (!qry.exec()) {
        m_lastError = qry.lastError().text();
        return QSqlQuery();
    }

    return qry;
}

QStringList DatabaseManager::getTables(const QString &connectionName)
{
    if (m_tablesCache.contains(connectionName)) {
        return m_tablesCache[connectionName];
    }

    if (!m_connections.contains(connectionName)) {
        return QStringList();
    }

    QSqlDatabase db = m_connections[connectionName];
    if (!db.isOpen()) {
        return QStringList();
    }

    QStringList tables = db.tables(QSql::Tables);
    m_tablesCache[connectionName] = tables;
    return tables;
}

QStringList DatabaseManager::getTableColumns(const QString &tableName, const QString &connectionName)
{
    QStringList columns;
    
    if (!m_connections.contains(connectionName)) {
        return columns;
    }

    QSqlDatabase db = m_connections[connectionName];
    if (!db.isOpen()) {
        return columns;
    }

    // Явное создание QSqlRecord через запрос
    QSqlQuery query(db);
    query.exec(QString("SELECT * FROM %1 LIMIT 1").arg(tableName));
    QSqlRecord record = query.record();
    
    for (int i = 0; i < record.count(); ++i) {
        columns << record.fieldName(i);
    }
    
    return columns;
}

QStringList DatabaseManager::activeConnections() const
{
    return m_connections.keys();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::beginTransaction(const QString &connectionName)
{
    if (!m_connections.contains(connectionName)) return false;
    return m_connections[connectionName].transaction();
}

bool DatabaseManager::commitTransaction(const QString &connectionName)
{
    if (!m_connections.contains(connectionName)) return false;
    return m_connections[connectionName].commit();
}

bool DatabaseManager::rollbackTransaction(const QString &connectionName)
{
    if (!m_connections.contains(connectionName)) return false;
    return m_connections[connectionName].rollback();
}