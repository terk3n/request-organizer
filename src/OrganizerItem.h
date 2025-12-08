#ifndef ORGANIZERITEM_H
#define ORGANIZERITEM_H

#include <QString>
#include <QColor>
#include <QList>
#include <QVariant>
#include <QtGlobal>
#include <QDateTime>

enum class ItemType {
    Folder,
    Request
};

class OrganizerItem {
public:
    explicit OrganizerItem(ItemType type, const QString& name, OrganizerItem* parent = nullptr);
    ~OrganizerItem();

    void appendChild(OrganizerItem* child);
    void insertChild(int position, OrganizerItem* child);
    void removeChild(OrganizerItem* child);
    void removeChild(int index);
    OrganizerItem* takeChild(int index);
    OrganizerItem* child(int row);
    const QList<OrganizerItem*>& children() const { return m_children; }
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool setData(int column, const QVariant& value);
    int row() const;
    OrganizerItem* parent();
    const OrganizerItem* parent() const;
    void setParent(OrganizerItem* parent) { m_parent = parent; }

    ItemType type() const { return m_type; }
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    
    QString annotation() const { return m_annotation; }
    void setAnnotation(const QString& annotation) { m_annotation = annotation; }
    
    QColor color() const { return m_color; }
    void setColor(const QColor& color) { m_color = color; }
    
    QString requestDetails() const { return m_requestDetails; }
    void setRequestDetails(const QString& details) { m_requestDetails = details; }
    
    int dbId() const { return m_dbId; }
    void setDbId(int id) { m_dbId = id; }
    
    QString request() const { return m_request; }
    void setRequest(const QString& request) { m_request = request; }
    
    QString response() const { return m_response; }
    void setResponse(const QString& response) { m_response = response; }
    
    QString host() const { return m_host; }
    void setHost(const QString& host) { m_host = host; }
    
    QString url() const { return m_url; }
    void setUrl(const QString& url) { m_url = url; }
    
    QString method() const { return m_method; }
    void setMethod(const QString& method) { m_method = method; }
    
    qint64 responseTime() const { return m_responseTime; }
    void setResponseTime(qint64 time) { m_responseTime = time; }
    
    QString query() const { return m_query; }
    void setQuery(const QString& query) { m_query = query; }
    
    int status() const { return m_status; }
    void setStatus(int status) { m_status = status; }
    
    qint64 length() const { return m_length; }
    void setLength(qint64 length) { m_length = length; }
    
    qint64 timestamp() const { return m_timestamp; }
    void setTimestamp(qint64 timestamp) { m_timestamp = timestamp; }
    
    QString screenshot() const { return m_screenshot; }
    void setScreenshot(const QString& screenshot) { m_screenshot = screenshot; }
    bool hasScreenshot() const { return !m_screenshot.isEmpty(); }

    bool isExpanded() const { return m_expanded; }
    void setExpanded(bool expanded) { m_expanded = expanded; }

private:
    ItemType m_type;
    QString m_name;
    QString m_annotation;
    QColor m_color;
    QString m_requestDetails;
    bool m_expanded;
    int m_dbId;
    QString m_request;
    QString m_response;
    QString m_host;
    QString m_url;
    QString m_method;
    qint64 m_responseTime;
    QString m_query;
    int m_status;
    qint64 m_length;
    qint64 m_timestamp;
    QString m_screenshot;
    
    OrganizerItem* m_parent;
    QList<OrganizerItem*> m_children;
};

#endif // ORGANIZERITEM_H
