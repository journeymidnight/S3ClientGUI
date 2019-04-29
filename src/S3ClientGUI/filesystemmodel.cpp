#include "filesystemmodel.h"
#include <QFileIconProvider>

FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
}

//QModelIndex FileSystemModel::index(int row, int column, const QModelIndex &parent) const
//{
//    // FIXME: Implement me!
//  return QFileSystemModel::index(row, column, parent);
//}
//
//QModelIndex FileSystemModel::parent(const QModelIndex &index) const
//{
//    // FIXME: Implement me!
//}
//
//int FileSystemModel::rowCount(const QModelIndex &parent) const
//{
//    //if (!parent.isValid())
//    //    return 0;
//
//    // FIXME: Implement me!
//}
//
//int FileSystemModel::columnCount(const QModelIndex &parent) const
//{
//    if (!parent.isValid())
//        return 0;
//
//    // FIXME: Implement me!
//}
//
//bool FileSystemModel::hasChildren(const QModelIndex &parent) const
//{
//    // FIXME: Implement me!
//}
//
//bool FileSystemModel::canFetchMore(const QModelIndex &parent) const
//{
//    // FIXME: Implement me!
//    return false;
//}
//
//void FileSystemModel::fetchMore(const QModelIndex &parent)
//{
//    // FIXME: Implement me!
//}
//
QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();



    // FIXME: Implement me!
    if (index.column() == 0) {
        if (role == Qt::DecorationRole) {
            //parent dir ".." should display folder icon
            if (index.data().toString() == "..")
                return iconProvider()->icon(QFileIconProvider::Folder);
        }
    } else {
        // if Line#0 equal "..", do not show Size/Type/Date Modified
        if (index.row() == 0/* && index.column() != 2*/) {
            QModelIndex idx = QFileSystemModel::index(0, 0, index.parent());
            if (QFileSystemModel::data(idx).toString() == "..")
                return QVariant();
        }
    }

    return QFileSystemModel::data(index, role);
}
