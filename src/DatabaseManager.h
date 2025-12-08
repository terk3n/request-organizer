#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QColor>
#include "OrganizerItem.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();
    bool initialize();
    int saveItem(int id, ItemType type, const QString& name, const QString& annotation, 
                  const QColor& color, const QString& request, const QString& response, int parentId,
                  const QString& host = "", const QString& url = "", const QString& method = "", qint64 responseTime = 0,
                  const QString& query = "", int status = 0, qint64 length = 0, qint64 timestamp = 0,
                  const QString& screenshot = "");
    bool loadItems();
    bool deleteItem(int id);
    int getNextId();
    
    QSqlDatabase& database() { return m_database; }

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    bool createTables();
    QString m_dbPath;
    QSqlDatabase m_database;
};

#endif // DATABASEMANAGER_H
