#include "DatabaseManager.h"
#include "OrganizerItem.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlError>

DatabaseManager::DatabaseManager() {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    m_dbPath = dataPath + "/requests.db";
}

DatabaseManager::~DatabaseManager() {
    if (m_database.isOpen()) {
        m_database.close();
    }
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize() {
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(m_dbPath);
    
    if (!m_database.open()) {
        qDebug() << "Error opening database:" << m_database.lastError().text();
        return false;
    }
    
    return createTables();
}

bool DatabaseManager::createTables() {
    QSqlQuery query(m_database);
    
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type INTEGER NOT NULL,
            name TEXT NOT NULL,
            annotation TEXT,
            color TEXT,
            request TEXT,
            response TEXT,
            parent_id INTEGER,
            host TEXT,
            url TEXT,
            method TEXT,
            response_time INTEGER,
            query TEXT,
            status INTEGER,
            length INTEGER,
            timestamp INTEGER,
            screenshot TEXT,
            FOREIGN KEY (parent_id) REFERENCES items(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createTable)) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }
    
    // Add new columns if they don't exist (for existing databases)
    QStringList textColumns = {"host", "url", "method", "query", "screenshot"};
    QStringList intColumns = {"response_time", "status", "length", "timestamp"};
    
    for (const QString& column : textColumns) {
        QString alterTable = QString("ALTER TABLE items ADD COLUMN %1 TEXT").arg(column);
        query.exec(alterTable); // Ignore errors if column already exists
    }
    
    for (const QString& column : intColumns) {
        QString alterTable = QString("ALTER TABLE items ADD COLUMN %1 INTEGER").arg(column);
        query.exec(alterTable); // Ignore errors if column already exists
    }
    
    if (!query.exec(createTable)) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }
    
    return true;
}

int DatabaseManager::saveItem(int id, ItemType type, const QString& name, const QString& annotation,
                               const QColor& color, const QString& request, const QString& response, int parentId,
                               const QString& host, const QString& url, const QString& method, qint64 responseTime,
                               const QString& query, int status, qint64 length, qint64 timestamp,
                               const QString& screenshot) {
    QSqlQuery queryObj(m_database);
    
    if (id == -1) {
        // Insert new item
        queryObj.prepare("INSERT INTO items (type, name, annotation, color, request, response, parent_id, host, url, method, response_time, query, status, length, timestamp, screenshot) "
                     "VALUES (:type, :name, :annotation, :color, :request, :response, :parent_id, :host, :url, :method, :response_time, :query, :status, :length, :timestamp, :screenshot)");
    } else {
        // Update existing item
        queryObj.prepare("UPDATE items SET type = :type, name = :name, annotation = :annotation, "
                     "color = :color, request = :request, response = :response, parent_id = :parent_id, "
                     "host = :host, url = :url, method = :method, response_time = :response_time, "
                     "query = :query, status = :status, length = :length, timestamp = :timestamp, screenshot = :screenshot "
                     "WHERE id = :id");
        queryObj.bindValue(":id", id);
    }
    
    queryObj.bindValue(":type", static_cast<int>(type));
    queryObj.bindValue(":name", name);
    queryObj.bindValue(":annotation", annotation);
    queryObj.bindValue(":color", color.name());
    queryObj.bindValue(":request", request);
    queryObj.bindValue(":response", response);
    queryObj.bindValue(":parent_id", parentId == -1 ? QVariant() : parentId);
    queryObj.bindValue(":host", host);
    queryObj.bindValue(":url", url);
    queryObj.bindValue(":method", method);
    queryObj.bindValue(":response_time", responseTime);
    queryObj.bindValue(":query", query);
    queryObj.bindValue(":status", status);
    queryObj.bindValue(":length", length);
    queryObj.bindValue(":timestamp", timestamp);
    queryObj.bindValue(":screenshot", screenshot);
    
    if (!queryObj.exec()) {
        qDebug() << "Error saving item:" << queryObj.lastError().text();
        return -1;
    }
    
    if (id == -1) {
        // Get the generated ID
        return queryObj.lastInsertId().toInt();
    }
    
    return id;
}

bool DatabaseManager::loadItems() {
    QSqlQuery query("SELECT id, type, name, annotation, color, request, response, parent_id FROM items ORDER BY id", m_database);
    
    if (!query.exec()) {
        qDebug() << "Error loading items:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::deleteItem(int id) {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM items WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qDebug() << "Error deleting item:" << query.lastError().text();
        return false;
    }
    
    return true;
}

int DatabaseManager::getNextId() {
    QSqlQuery query("SELECT MAX(id) FROM items", m_database);
    if (query.next()) {
        return query.value(0).toInt() + 1;
    }
    return 1;
}
