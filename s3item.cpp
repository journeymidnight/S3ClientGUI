#include "s3item.h"

S3Item::S3Item(const QList<QVariant> &data, S3Type type, S3Item *parentItem) {
    m_parentItem = parentItem;
    m_type = type;
    m_itemData = data;
}


S3Item::~S3Item(){
    qDeleteAll(m_childItems);
}


void S3Item::appendChild(S3Item *child) {
    m_childItems.append(child);
}

S3Item* S3Item::child(int row) {
    return m_childItems.value(row);
}

int S3Item::childCount() const {
    return m_childItems.count();
}

S3Item * S3Item::parentItem(){
    return m_parentItem;
}

QVariant S3Item::data(int column) {
    return m_itemData.value(column);
}
