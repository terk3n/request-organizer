#include "OrganizerModel.h"
#include <QSqlQuery>
#include <QDebug>
#include <QDataStream>
#include <QMimeData>
#include <QByteArray>
#include <QIODevice>
#include <QSet>

OrganizerModel::OrganizerModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new OrganizerItem(ItemType::Folder, "Root");
    
    if (DatabaseManager::instance().initialize()) {
        loadItemsFromDatabase();
    }
}

OrganizerModel::~OrganizerModel() {
    delete m_rootItem;
}

QVariant OrganizerModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    OrganizerItem* item = static_cast<OrganizerItem*>(index.internalPointer());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return item->data(index.column());
    } else if (role == Qt::BackgroundRole) {
        if (item->type() == ItemType::Request && item->color() != Qt::white) {
            return item->color();
        }
    } else if (role == Qt::ForegroundRole) {
        if (item->type() == ItemType::Request && item->color() != Qt::white) {
            QColor bgColor = item->color();
            int brightness = (bgColor.red() + bgColor.green() + bgColor.blue()) / 3;
            return brightness < 128 ? QColor(Qt::white) : QColor(Qt::black);
        }
    }

    return QVariant();
}

bool OrganizerModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role != Qt::EditRole)
        return false;

    OrganizerItem* item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result) {
        saveItemToDatabase(item, getParentDbId(index.parent()));
        // Emit dataChanged for all columns if it's a request item and we're editing request-specific fields
        // This ensures the view updates properly
        if (item->type() == ItemType::Request && index.column() >= 2 && index.column() <= 9) {
            QModelIndex startIndex = this->index(index.row(), 0, index.parent());
            QModelIndex endIndex = this->index(index.row(), columnCount() - 1, index.parent());
            emit dataChanged(startIndex, endIndex, {role});
        } else {
            emit dataChanged(index, index, {role});
        }
    }

    return result;
}

Qt::ItemFlags OrganizerModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    
    OrganizerItem* item = getItem(index);
    if (item && item->type() == ItemType::Folder) {
        flags |= Qt::ItemIsDropEnabled;
    }
    
    return flags;
}

QVariant OrganizerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return "Name";
            case 1:
                return "Annotation";
            case 2:
                return "Host";
            case 3:
                return "URL";
            case 4:
                return "Method";
            case 5:
                return "Query";
            case 6:
                return "Status";
            case 7:
                return "Length";
            case 8:
                return "Response Time";
            case 9:
                return "Timestamp";
            case 10:
                return "Details";
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QModelIndex OrganizerModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    OrganizerItem* parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<OrganizerItem*>(parent.internalPointer());

    OrganizerItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex OrganizerModel::parent(const QModelIndex& index) const {
    if (!index.isValid())
        return QModelIndex();

    OrganizerItem* childItem = static_cast<OrganizerItem*>(index.internalPointer());
    OrganizerItem* parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int OrganizerModel::rowCount(const QModelIndex& parent) const {
    OrganizerItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<OrganizerItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int OrganizerModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 11; // Name, Annotation, Host, URL, Method, Query, Status, Length, Response Time, Timestamp, Details
}

bool OrganizerModel::insertRows(int row, int count, const QModelIndex& parent) {
    OrganizerItem* parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, row, row + count - 1);
    success = true;
    endInsertRows();

    return success;
}

namespace {
    void deleteItemRecursive(OrganizerItem* item, DatabaseManager& db, QMap<int, OrganizerItem*>& itemsById) {
        if (!item) return;
        
        // Delete all children first
        for (int i = item->childCount() - 1; i >= 0; --i) {
            deleteItemRecursive(item->child(i), db, itemsById);
        }
        
        // Delete from database
        if (item->dbId() != -1) {
            db.deleteItem(item->dbId());
            itemsById.remove(item->dbId());
        }
    }
}

bool OrganizerModel::removeRows(int row, int count, const QModelIndex& parent) {
    OrganizerItem* parentItem = getItem(parent);
    
    // Check if any of the items being removed are currently being moved
    QList<int> movedDbIds;
    for (int i = 0; i < count && (row + i) < parentItem->childCount(); ++i) {
        OrganizerItem* child = parentItem->child(row + i);
        if (child && child->dbId() != -1 && m_itemsBeingMoved.contains(child->dbId())) {
            movedDbIds.append(child->dbId());
        }
    }
    
    // If items are being moved, don't delete them - they've already been moved
    if (!movedDbIds.isEmpty()) {
        // Items are being moved, they've already been removed from parent in dropMimeData
        // Unmark them and return true
        for (int dbId : movedDbIds) {
            m_itemsBeingMoved.remove(dbId);
        }
        return true;
    }

    // Normal deletion
    bool success = true;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        if (row >= parentItem->childCount()) break; // Safety check
        OrganizerItem* child = parentItem->child(row);
        if (child) {
            deleteItemRecursive(child, DatabaseManager::instance(), m_itemsById);
            parentItem->removeChild(row);
        } else {
            parentItem->removeChild(row);
        }
    }
    endRemoveRows();

    return success;
}

OrganizerItem* OrganizerModel::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        OrganizerItem* item = static_cast<OrganizerItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_rootItem;
}

QModelIndex OrganizerModel::addFolder(const QString& name, const QModelIndex& parent) {
    OrganizerItem* parentItem = getItem(parent);
    int row = parentItem->childCount();

    beginInsertRows(parent, row, row);
    OrganizerItem* folder = new OrganizerItem(ItemType::Folder, name, parentItem);
    parentItem->appendChild(folder);
    saveItemToDatabase(folder, getParentDbId(parent));
    endInsertRows();

    return index(row, 0, parent);
}

QModelIndex OrganizerModel::addRequest(const QString& name, const QModelIndex& parent) {
    OrganizerItem* parentItem = getItem(parent);
    int row = parentItem->childCount();

    beginInsertRows(parent, row, row);
    OrganizerItem* request = new OrganizerItem(ItemType::Request, name, parentItem);
    parentItem->appendChild(request);
    saveItemToDatabase(request, getParentDbId(parent));
    endInsertRows();

    return index(row, 0, parent);
}

void OrganizerModel::setRequest(const QModelIndex& index, const QString& request) {
    OrganizerItem* item = getItem(index);
    if (item && item->type() == ItemType::Request) {
        item->setRequest(request);
        saveItemToDatabase(item, getParentDbId(index.parent()));
        emit dataChanged(index, index);
    }
}

void OrganizerModel::setResponse(const QModelIndex& index, const QString& response) {
    OrganizerItem* item = getItem(index);
    if (item && item->type() == ItemType::Request) {
        item->setResponse(response);
        saveItemToDatabase(item, getParentDbId(index.parent()));
        emit dataChanged(index, index);
    }
}

void OrganizerModel::loadItemsFromDatabase() {
    QSqlQuery query("SELECT id, type, name, annotation, color, request, response, parent_id, "
                    "COALESCE(host, '') as host, COALESCE(url, '') as url, "
                    "COALESCE(method, '') as method, COALESCE(response_time, 0) as response_time, "
                    "COALESCE(query, '') as query, COALESCE(status, 0) as status, "
                    "COALESCE(length, 0) as length, COALESCE(timestamp, 0) as timestamp, "
                    "COALESCE(screenshot, '') as screenshot "
                    "FROM items ORDER BY id", 
                    DatabaseManager::instance().database());
    
    QMap<int, OrganizerItem*> itemsMap;
    QMap<int, int> parentMap; // Maps item id to parent id
    
    // First pass: create all items
    while (query.next()) {
        int id = query.value(0).toInt();
        ItemType type = static_cast<ItemType>(query.value(1).toInt());
        QString name = query.value(2).toString();
        QString annotation = query.value(3).toString();
        QColor color = QColor(query.value(4).toString());
        QString request = query.value(5).toString();
        QString response = query.value(6).toString();
        int parentId = query.value(7).toInt();
        QString host = query.value(8).toString();
        QString url = query.value(9).toString();
        QString method = query.value(10).toString();
        qint64 responseTime = query.value(11).toLongLong();
        QString queryStr = query.value(12).toString();
        int status = query.value(13).toInt();
        qint64 length = query.value(14).toLongLong();
        qint64 timestamp = query.value(15).toLongLong();
        QString screenshot = query.value(16).toString();
        
        OrganizerItem* item = new OrganizerItem(type, name);
        item->setDbId(id);
        item->setAnnotation(annotation);
        item->setColor(color);
        item->setRequest(request);
        item->setResponse(response);
        item->setHost(host);
        item->setUrl(url);
        item->setMethod(method);
        item->setResponseTime(responseTime);
        item->setQuery(queryStr);
        item->setStatus(status);
        item->setLength(length);
        item->setTimestamp(timestamp);
        item->setScreenshot(screenshot);
        
        itemsMap[id] = item;
        m_itemsById[id] = item;
        parentMap[id] = parentId;
    }
    
    // Second pass: establish parent-child relationships
    for (auto it = itemsMap.begin(); it != itemsMap.end(); ++it) {
        int id = it.key();
        OrganizerItem* item = it.value();
        int parentId = parentMap[id];
        
        if (parentId == 0 || !itemsMap.contains(parentId)) {
            // Root level item
            m_rootItem->appendChild(item);
        } else {
            // Child item
            OrganizerItem* parent = itemsMap[parentId];
            parent->appendChild(item);
        }
    }
}

void OrganizerModel::saveItemToDatabase(OrganizerItem* item, int parentDbId) {
    if (!item) return;
    
    QString requestBase64 = item->request();
    QString responseBase64 = item->response();
    
    int dbId = item->dbId();
    int savedId = DatabaseManager::instance().saveItem(
        dbId,
        item->type(),
        item->name(),
        item->annotation(),
        item->color(),
        requestBase64,
        responseBase64,
        parentDbId,
        item->host(),
        item->url(),
        item->method(),
        item->responseTime(),
        item->query(),
        item->status(),
        item->length(),
        item->timestamp(),
        item->screenshot()
    );
    
    if (savedId != -1 && dbId == -1) {
        // New item, update with the generated ID
        item->setDbId(savedId);
        m_itemsById[savedId] = item;
    }
}

OrganizerItem* OrganizerModel::findItemByDbId(int dbId, OrganizerItem* start) {
    if (!start) start = m_rootItem;
    
    if (start->dbId() == dbId) {
        return start;
    }
    
    for (int i = 0; i < start->childCount(); ++i) {
        OrganizerItem* found = findItemByDbId(dbId, start->child(i));
        if (found) return found;
    }
    
    return nullptr;
}

int OrganizerModel::getParentDbId(const QModelIndex& parent) {
    if (!parent.isValid()) {
        return -1; // Root
    }
    
    OrganizerItem* parentItem = getItem(parent);
    return parentItem ? parentItem->dbId() : -1;
}

void OrganizerModel::saveItem(const QModelIndex& index) {
    if (!index.isValid()) return;
    
    OrganizerItem* item = getItem(index);
    int parentDbId = getParentDbId(index.parent());
    saveItemToDatabase(item, parentDbId);
}

QModelIndex OrganizerModel::findIndexForItem(OrganizerItem* item, const QModelIndex& parent) const {
    if (!item) return QModelIndex();
    
    OrganizerItem* parentItem = getItem(parent);
    if (!parentItem) return QModelIndex();
    
    // Check all children of the parent
    for (int i = 0; i < parentItem->childCount(); ++i) {
        OrganizerItem* child = parentItem->child(i);
        if (child == item) {
            return index(i, 0, parent);
        }
        // Recursively check children
        QModelIndex childIndex = index(i, 0, parent);
        QModelIndex found = findIndexForItem(item, childIndex);
        if (found.isValid()) {
            return found;
        }
    }
    
    return QModelIndex();
}

Qt::DropActions OrganizerModel::supportedDropActions() const {
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions OrganizerModel::supportedDragActions() const {
    return Qt::MoveAction;
}

QStringList OrganizerModel::mimeTypes() const {
    QStringList types;
    types << "application/x-organizer-item";
    return types;
}

QMimeData* OrganizerModel::mimeData(const QModelIndexList& indexes) const {
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex& index : indexes) {
        if (index.isValid() && index.column() == 0) {
            OrganizerItem* item = getItem(index);
            if (item) {
                // Store the row, parent row, and a unique identifier
                QModelIndex parent = index.parent();
                stream << index.row();
                stream << (parent.isValid() ? parent.row() : -1);
                stream << item->dbId();
            }
        }
    }

    mimeData->setData("application/x-organizer-item", encodedData);
    return mimeData;
}

bool OrganizerModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
    Q_UNUSED(column);
    
    if (!data->hasFormat("application/x-organizer-item"))
        return false;

    if (action != Qt::MoveAction)
        return false;

    // Can drop on folders or at root level
    if (parent.isValid()) {
        OrganizerItem* parentItem = getItem(parent);
        if (!parentItem || parentItem->type() != ItemType::Folder) {
            return false;
        }
    }

    return true;
}

bool OrganizerModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    if (action != Qt::MoveAction)
        return false;

    QByteArray encodedData = data->data("application/x-organizer-item");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    int sourceRow = -1;
    int sourceParentRow = -1;
    int dbId = -1;
    
    if (stream.atEnd())
        return false;

    stream >> sourceRow >> sourceParentRow >> dbId;

    // Find the source item
    OrganizerItem* sourceItem = m_itemsById.value(dbId, nullptr);
    if (!sourceItem)
        return false;

    // Find the source index
    QModelIndex sourceIndex = findIndexForItem(sourceItem);
    if (!sourceIndex.isValid()) {
        return false;
    }

    // Get source parent index using the model's parent() method
    QModelIndex sourceParent = sourceIndex.parent();

    // Get source parent item
    OrganizerItem* sourceParentItem = sourceItem->parent();
    if (!sourceParentItem) {
        sourceParentItem = m_rootItem;
    }

    // Get destination parent
    OrganizerItem* destParentItem = getItem(parent);
    if (!destParentItem) {
        destParentItem = m_rootItem;
    }

    // Check if destination is a folder
    if (destParentItem->type() != ItemType::Folder) {
        return false;
    }

    // Prevent dropping item into itself or its descendants
    if (destParentItem == sourceItem) {
        return false;
    }
    
    // Check if dropping into a descendant
    OrganizerItem* checkItem = destParentItem;
    while (checkItem && checkItem->parent()) {
        if (checkItem->parent() == sourceItem) {
            return false; // Cannot drop into a descendant
        }
        checkItem = checkItem->parent();
    }

    // Calculate destination row
    if (row == -1) {
        row = destParentItem->childCount();
    }

    // Verify sourceItem is actually a child of sourceParentItem
    int sourceRowActual = -1;
    for (int i = 0; i < sourceParentItem->childCount(); ++i) {
        if (sourceParentItem->child(i) == sourceItem) {
            sourceRowActual = i;
            break;
        }
    }
    if (sourceRowActual == -1) {
        return false;
    }

    // If moving to the same parent, adjust row if necessary
    if (sourceParentItem == destParentItem) {
        if (sourceRowActual < row) {
            row--; // Adjust for removal
        }
        if (sourceRowActual == row) {
            return false; // No change
        }
    }

    // Mark item as being moved to prevent deletion
    m_itemsBeingMoved.insert(dbId);
    
    // Use beginMoveRows/endMoveRows for proper notification
    if (!beginMoveRows(sourceParent, sourceRowActual, sourceRowActual, parent, row)) {
        m_itemsBeingMoved.remove(dbId);
        return false;
    }
    
    // Remove from source parent (without deleting)
    OrganizerItem* movedItem = sourceParentItem->takeChild(sourceRowActual);
    if (!movedItem || movedItem != sourceItem) {
        // If takeChild failed, restore and abort
        if (movedItem && movedItem != sourceItem) {
            sourceParentItem->insertChild(sourceRowActual, movedItem);
        } else if (movedItem) {
            sourceParentItem->insertChild(sourceRowActual, movedItem);
        }
        m_itemsBeingMoved.remove(dbId);
        endMoveRows();
        return false;
    }
    
    // Insert into destination parent
    destParentItem->insertChild(row, movedItem);
    
    endMoveRows();

    // Update database - save the moved item with new parent
    int parentDbId = destParentItem == m_rootItem ? -1 : destParentItem->dbId();
    saveItemToDatabase(movedItem, parentDbId);

    // Unmark item as being moved AFTER a short delay to ensure removeRows check happens
    // Actually, keep it marked until we're sure removeRows won't be called
    // We'll unmark it in removeRows if it gets called
    
    // Return true - Qt will handle the view update via beginMoveRows/endMoveRows
    return true;
}
