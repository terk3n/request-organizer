#ifndef ORGANIZERMODEL_H
#define ORGANIZERMODEL_H

#include <QAbstractItemModel>
#include <QColor>
#include <QMimeData>
#include <QSet>
#include "OrganizerItem.h"
#include "DatabaseManager.h"
#include <QSqlQuery>

class OrganizerModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit OrganizerModel(QObject* parent = nullptr);
    ~OrganizerModel();

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    // Drag and Drop support
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    OrganizerItem* rootItem() const { return m_rootItem; }
    OrganizerItem* getItem(const QModelIndex& index) const;
    QModelIndex addFolder(const QString& name, const QModelIndex& parent = QModelIndex());
    QModelIndex addRequest(const QString& name, const QModelIndex& parent = QModelIndex());
    void setRequest(const QModelIndex& index, const QString& request);
    void setResponse(const QModelIndex& index, const QString& response);
    void saveItem(const QModelIndex& index);

private:
    void loadItemsFromDatabase();
    void saveItemToDatabase(OrganizerItem* item, int parentDbId = -1);
    OrganizerItem* findItemByDbId(int dbId, OrganizerItem* start = nullptr);
    int getParentDbId(const QModelIndex& parent);
    QModelIndex findIndexForItem(OrganizerItem* item, const QModelIndex& parent = QModelIndex()) const;
    
    OrganizerItem* m_rootItem;
    QMap<int, OrganizerItem*> m_itemsById;
    QSet<int> m_itemsBeingMoved; // Track items currently being moved to prevent deletion
};

#endif // ORGANIZERMODEL_H
