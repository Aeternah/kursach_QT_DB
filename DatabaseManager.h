#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QHash>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    enum DatabaseType { SQLite, PostgreSQL };

    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool connectToDatabase(DatabaseType type, const QString &connectionName,
                         const QString &databaseName,
                         const QString &host = "",
                         const QString &user = "",
                         const QString &password = "",
                         int port = -1);

    void disconnectFromDatabase(const QString &connectionName);
    QSqlQuery executeQuery(const QString &query, const QString &connectionName);
    QStringList getTables(const QString &connectionName);
    QStringList getTableColumns(const QString &tableName, const QString &connectionName);
    QStringList activeConnections() const;
    QString lastError() const;

    bool beginTransaction(const QString &connectionName);
    bool commitTransaction(const QString &connectionName);
    bool rollbackTransaction(const QString &connectionName);

private:
    QHash<QString, QSqlDatabase> m_connections;
    QHash<QString, QStringList> m_tablesCache;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H