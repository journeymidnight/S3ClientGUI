#ifndef S3ITEM_H
#define S3ITEM_H
#include <QList>
#include <QVariant>

//From QT5 examples
enum S3Type {
    S3FileType,
    S3BucketType,
    S3DirectoryType,
};

class S3Item
{
public:
    explicit S3Item(const QList<QVariant> &data, S3Type type, S3Item *parentItem=0);
    ~S3Item();
    int row() const;
    int columnCount() const;
    S3Item *parentItem();
    int childCount() const;
    QVariant data(int column);
    void appendChild(S3Item *child);
    S3Item* child(int row);
private:
    QList<S3Item*> m_childItems;
    QList<QVariant> m_itemData;
    S3Item * m_parentItem;
    S3Type m_type;
};

#endif // S3ITEM_H
