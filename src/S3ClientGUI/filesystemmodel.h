#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QFileSystemModel>

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit FileSystemModel(QObject *parent = nullptr);

    // Basic functionality:
    //QModelIndex index(int row, int column,
    //const QModelIndex &parent = QModelIndex()) const override;
    //QModelIndex parent(const QModelIndex &index) const override;

    //int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //// Fetch data dynamically:
    //bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    //bool canFetchMore(const QModelIndex &parent) const override;
    //void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // FILESYSTEMMODEL_H
