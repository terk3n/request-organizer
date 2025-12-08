#include "OrganizerItem.h"

OrganizerItem::OrganizerItem(ItemType type, const QString& name, OrganizerItem* parent)
    : m_type(type)
    , m_name(name)
    , m_annotation("")
    , m_color(Qt::white)
    , m_requestDetails("")
    , m_expanded(true)
    , m_dbId(-1)
    , m_request("")
    , m_response("")
    , m_host("")
    , m_url("")
    , m_method("")
    , m_responseTime(0)
    , m_query("")
    , m_status(0)
    , m_length(0)
    , m_timestamp(0)
    , m_screenshot("")
    , m_parent(parent)
{
}

OrganizerItem::~OrganizerItem() {
    qDeleteAll(m_children);
}

void OrganizerItem::appendChild(OrganizerItem* child) {
    if (child) {
        child->setParent(this);
        m_children.append(child);
    }
}

void OrganizerItem::insertChild(int position, OrganizerItem* child) {
    if (child && position >= 0 && position <= m_children.size()) {
        child->setParent(this);
        m_children.insert(position, child);
    }
}

void OrganizerItem::removeChild(OrganizerItem* child) {
    int index = m_children.indexOf(child);
    if (index >= 0) {
        m_children.removeAt(index);
        delete child;
    }
}

void OrganizerItem::removeChild(int index) {
    if (index >= 0 && index < m_children.size()) {
        OrganizerItem* child = m_children.takeAt(index);
        delete child;
    }
}

OrganizerItem* OrganizerItem::takeChild(int index) {
    if (index >= 0 && index < m_children.size()) {
        OrganizerItem* child = m_children.takeAt(index);
        child->setParent(nullptr);
        return child;
    }
    return nullptr;
}

OrganizerItem* OrganizerItem::child(int row) {
    if (row < 0 || row >= m_children.size())
        return nullptr;
    return m_children.at(row);
}

int OrganizerItem::childCount() const {
    return m_children.size();
}

int OrganizerItem::columnCount() const {
    return 11; // Name, Annotation, Host, URL, Method, Query, Status, Length, Response Time, Timestamp, Details
}

QVariant OrganizerItem::data(int column) const {
    switch (column) {
        case 0:
            return m_name;
        case 1:
            return m_annotation;
        case 2:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            return m_host;
        case 3:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            return m_url;
        case 4:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            return m_method;
        case 5:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            return m_query;
        case 6:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            if (m_status > 0) {
                return m_status;
            }
            return QVariant();
        case 7:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            if (m_length > 0) {
                return QString("%1 bytes").arg(m_length);
            }
            return QVariant();
        case 8:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            if (m_responseTime > 0) {
                return QString("%1 ms").arg(m_responseTime);
            }
            return QVariant();
        case 9:
            if (m_type == ItemType::Folder) {
                return QVariant();
            }
            if (m_timestamp > 0) {
                QDateTime dateTime = QDateTime::fromSecsSinceEpoch(m_timestamp);
                return dateTime.toString("yyyy-MM-dd hh:mm:ss");
            }
            return QVariant();
        case 10:
            if (m_type == ItemType::Folder) {
                return QString("%1 items").arg(m_children.size());
            } else {
                return m_requestDetails.isEmpty() ? "Request" : m_requestDetails;
            }
        default:
            return QVariant();
    }
}

bool OrganizerItem::setData(int column, const QVariant& value) {
    switch (column) {
        case 0:
            m_name = value.toString();
            return true;
        case 1:
            m_annotation = value.toString();
            return true;
        case 2:
            if (m_type == ItemType::Request) {
                m_host = value.toString();
                return true;
            }
            return false;
        case 3:
            if (m_type == ItemType::Request) {
                m_url = value.toString();
                return true;
            }
            return false;
        case 4:
            if (m_type == ItemType::Request) {
                m_method = value.toString();
                return true;
            }
            return false;
        case 5:
            if (m_type == ItemType::Request) {
                m_query = value.toString();
                return true;
            }
            return false;
        case 6:
            if (m_type == ItemType::Request) {
                bool ok;
                int status = value.toInt(&ok);
                if (ok) {
                    m_status = status;
                }
                return ok;
            }
            return false;
        case 7:
            if (m_type == ItemType::Request) {
                QString lengthStr = value.toString();
                lengthStr.remove(" bytes");
                bool ok;
                qint64 length = lengthStr.toLongLong(&ok);
                if (ok) {
                    m_length = length;
                }
                return ok;
            }
            return false;
        case 8:
            if (m_type == ItemType::Request) {
                QString timeStr = value.toString();
                timeStr.remove(" ms");
                bool ok;
                qint64 time = timeStr.toLongLong(&ok);
                if (ok) {
                    m_responseTime = time;
                }
                return ok;
            }
            return false;
        case 9:
            if (m_type == ItemType::Request) {
                QDateTime dateTime = QDateTime::fromString(value.toString(), "yyyy-MM-dd hh:mm:ss");
                if (dateTime.isValid()) {
                    m_timestamp = dateTime.toSecsSinceEpoch();
                    return true;
                }
                return false;
            }
            return false;
        case 10:
            if (m_type == ItemType::Request) {
                m_requestDetails = value.toString();
                return true;
            }
            return false;
        default:
            return false;
    }
}

int OrganizerItem::row() const {
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<OrganizerItem*>(this));
    }
    return 0;
}

OrganizerItem* OrganizerItem::parent() {
    return m_parent;
}

const OrganizerItem* OrganizerItem::parent() const {
    return m_parent;
}
