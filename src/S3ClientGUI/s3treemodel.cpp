#include <QMimeDatabase>
#include <QSize>
#include <QIcon>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <shellapi.h>
#include <QtWin>

#ifdef GetMessage
//Aws::Client::AWSError::GetMessage() conflict with win32 lib
#undef GetMessage
#endif // GetMessage

//Aws::S3::S3Client::GetObject() conflict with win32 lib
#ifdef GetObject
#undef GetObject
#endif // GetObject
#endif // Q_OS_WIN

#include "s3treemodel.h"
#include "helper.h"


static void advanceSpinner(QVariantList &loadData)
{
    QVariantList text = { "...loading [-]", "...loading [\\]", "...loading [|]", "...loading [/]" };
    for (int i = 0; i < text.size(); ++i) {
        if (loadData.first() == text[i]) {
            i = (i + 1) % text.size();
            loadData.first() = text[i];
            break;
        }
    }
}

S3TreeModel::S3TreeModel(QS3Client *s3client, QObject *parent): QAbstractItemModel(parent),
    m_s3client(s3client)
    , m_truncated(false)
    , m_deleteFinishedCnt(0)
{
    m_titles << "Name" << "Mtime" << "Owner" << "Size";

    /*
    connect(m_s3client, SIGNAL(ListBucketInfo(s3bucket)), this, SLOT(listBucketInfo(s3bucket)));
    connect(m_s3client, SIGNAL(ListObjectInfo(s3object, QString)), this, SLOT(listObjectInfo(s3object, QString)));
    connect(m_s3client, SIGNAL(ListPrefixInfo(s3prefix, QString)), this, SLOT(listPrefixInfo(s3prefix, QString)));


    connect(m_s3client, SIGNAL(ListBucketFinished(bool, s3error)), this, SLOT(listBucketFinishd(bool,s3error)));
    connect(m_s3client, SIGNAL(ListObjectFinished(bool, s3error, bool)), this, SLOT(listObjectFinished(bool,s3error,bool)));
    */
}


S3TreeModel::~S3TreeModel()
{
    qDeleteAll(m_currentData);
    /*m_s3client will be cleaned by others*/
}



void S3TreeModel::setRootIndex(const QModelIndex &index)
{
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());

    qDebug() << "Got item:" << item;
    QString path;
    if (item->type == S3FileType) {
        //do not expand
        return;
    } else if (item->type == S3BucketType) {
        path = "/" + item->objectPath + "/";
    } else if (item->type == S3DirectoryType ||
               item->type == S3ParentDirectoryType) {
        path = "/" + item->bucketName + "/" + item->objectPath;
    }
    qDebug() << "PATH NAME" << path;
    setRootPath(path);
}

/*
 *  Path Example = /bucketName/dirName/
 *  Path Example = /
 *  Path Example = /bucketName/
 *  Path Example = /bucketName/../
 *  Path Example = /bucketName/dirName/../
 */

// parse ".." of path
QString S3TreeModel::toValidPath(QString path)
{
    QString validPath = path;
    QStringList parts = path.split('/');

    //path start with a '/' and end with '/'
    if (parts.count() >= 2 && parts.last() == "") {
    } else {
        qDebug() << "bad path name:" << path;
        return validPath;
    }

    if (!parts.contains(dotdot))
        return validPath;

    if (!parts.isEmpty())
        parts.removeLast();
    if (!parts.isEmpty())
        parts.removeLast();
    if (!parts.isEmpty())
        parts.removeLast();

    validPath = parts.join(QChar('/'));
    validPath.append(QChar('/'));
    return validPath;
}

void S3TreeModel::setRootPath(const QString &path)
{
    qDeleteAll(m_currentData);
    m_currentData.clear();

    //loading
    QVariantList loadData;
    loadData << "...loading [-]" << QVariant() << QVariant() << QVariant();
    m_currentData.append(new SimpleItem(loadData, S3LoadingType, "", ""));

    QObject::connect(timer, &QTimer::timeout, this, [ = ]() {
        beginResetModel();
        advanceSpinner(m_currentData.first()->data);
        endResetModel();
    });
    timer->start(70);
    //In commandFinished slot, I will call endResetModel;
    beginResetModel();

    QString validPath = toValidPath(path);
    QStringList parts = validPath.split("/");
    if (parts.length() >= 2 && !parts[1].isEmpty())
        emit currentViewIsBucket(false);
    else
        emit currentViewIsBucket(true);

    m_currentPath = validPath;
    QString bucketName = parts[1];
    qDebug() << "bucket name:" << bucketName;
    if (bucketName == "") {
        //listBucketInfo will fill the m_currentData;

        ListBucketAction *lbAction = m_s3client->ListBuckets();
        connect(lbAction, SIGNAL(ListBucketInfo(s3bucket)), this, SLOT(listBucketInfo(s3bucket)));
        connect(lbAction, SIGNAL(ListBucketFinished(bool, s3error)), this, SLOT(listBucketFinishd(bool,
                                                                                                  s3error)));

        /*
        connect(lbAction, &ListBucketAction::finished, this, [=](){
            //I wish the signal could be disconnect automatically.
            lbAction->deleteLater();
            emit this->cmdFinished();
        });
        */

        return;
    }

    QString prefix;
    // start from bucketName, end before the last ""
    for (int i = 2; i < parts.size() - 1; i++) {
        prefix.append(parts[i]);
        prefix.append("/");
    }

    qDebug() << "prefix name" << prefix;
    //in begining, insert ".." Folder
    QVariantList data;
    data << dotdot << QVariant() << QVariant() << QVariant();
    m_tempData = new SimpleItem(data, S3ParentDirectoryType, bucketName, prefix + dotdot + QChar('/'));
    //listObjectInfo will fill the m_currentData;
    ListObjectAction *loAction  = m_s3client->ListObjects(bucketName, "", prefix, QString('/'));

    connect(loAction, SIGNAL(ListObjectInfo(s3object, QString)), this, SLOT(listObjectInfo(s3object,
                                                                                           QString)));
    connect(loAction, SIGNAL(ListPrefixInfo(s3prefix, QString)), this, SLOT(listPrefixInfo(s3prefix,
                                                                                           QString)));
    connect(loAction, SIGNAL(ListObjectFinished(bool, s3error, bool, QString)), this,
            SLOT(listObjectFinished(bool, s3error, bool, QString)));



    return;
}

void S3TreeModel::refresh()
{
    setRootPath(getRootPath());
}

void S3TreeModel::listBucketInfo(s3bucket bucket)
{
    QList<QVariant> data;
    QString bucketName = AwsString2QString(bucket.GetName());
    data << bucketName << AwsString2QString(bucket.GetCreationDate().ToLocalTimeString("%Y/%m/%d %R"))
         << "" << "";
    m_currentData.append(new SimpleItem(data, S3BucketType, QString(), bucketName));
}


void S3TreeModel::listObjectInfo(s3object object, QString bucketName)
{
    QList<QVariant> data;

    /*key should be ZZZ/b.pdf */
    QString key = AwsString2QString(object.GetKey());

    if (key[key.size() - 1] == '/' && object.GetSize() == 0) {
        //a simulated directoy
        return;
    }

    //Trim key based on the prefix for display
    QString name;
    int pos = key.lastIndexOf('/');
    if (pos != -1) {
        name = key.mid(pos + 1, -1);
    } else {
        name = key;
    }

    data << name << AwsString2QString(object.GetLastModified().ToLocalTimeString("%Y/%m/%d %R"))
         << AwsString2QString(object.GetOwner().GetDisplayName()) << helper::formattedDataSize(
             object.GetSize(), 1);

    //
    data << AwsString2QString(object.GetETag()) << AwsString2QString(
             Aws::S3::Model::ObjectStorageClassMapper::GetNameForObjectStorageClass(object.GetStorageClass()));

    m_currentData.append(new SimpleItem(data, S3FileType, bucketName, key));
}


void S3TreeModel::listPrefixInfo(s3prefix prefix, QString bucketName)
{
    QList<QVariant> data;
    /*prefixName should be ZZZ/b/ */
    QString prefixName = AwsString2QString(prefix.GetPrefix());

    qDebug() << prefixName;
    //Trim key based on the prefix for display
    QString name = prefixName;
    name.chop(1);//remote last slash'/'
    int pos = name.lastIndexOf('/');
    if (pos != -1) {
        name = name.mid(pos + 1, -1);
    }

    data << name;
    //placeholder
    data << QVariant() << QVariant() << QVariant();
    m_currentData.append(new SimpleItem(data, S3DirectoryType, bucketName, prefixName));
}

void S3TreeModel::listBucketFinishd(bool success, s3error error)
{
    if (!success) {
        qDebug() << "list bucket error!";
    }

    timer->stop();
    m_currentData.removeAt(0);

    endResetModel();
    emit rootPathChanged(m_currentPath);
    emit updateInfo("good");

    auto action = qobject_cast<ListBucketAction *>(sender());
    action->deleteLater();
    emit this->cmdFinished(success, error);
}

void S3TreeModel::listObjectFinished(bool success, s3error error, bool truncated,
                                     QString nextMarker)
{
    Q_UNUSED(nextMarker);

    if (!success) {
        qDebug() << "list object error!" << AwsString2QString(error.GetMessage());
    }
    m_truncated = truncated;
    if (m_currentData.size() == 0) {
        emit updateInfo("empty dir");
    } else {
        emit updateInfo("good");
    }
    emit rootPathChanged(m_currentPath);

    timer->stop();
    m_currentData[0] = m_tempData;
    endResetModel();


    //some clear up
    auto action = qobject_cast<ListObjectAction *>(sender());
    action->deleteLater();
    emit this->cmdFinished(success, error);
}


/* pure virtual functions */
QModelIndex S3TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (m_currentData.size() > row)
        return createIndex(row, column, m_currentData[row]);
    else
        return QModelIndex();
}


QVariant S3TreeModel::data(const QModelIndex &index, int role) const
{
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());

    if (role == Qt::DecorationRole && index.column() == 0) {
        if (index.column() == 0) {
            switch (item->type) {
            case S3FileType: {
                QIcon icon = QIcon();
#ifdef Q_OS_WIN
                SHFILEINFOA info;
                ZeroMemory(&info, sizeof(SHFILEINFOA));
                QString fileExtension = item->data.value(index.column()).toString();
                fileExtension = fileExtension.mid(fileExtension.lastIndexOf('.'));
                if (SHGetFileInfoA(fileExtension.toStdString().c_str(),
                                   FILE_ATTRIBUTE_NORMAL,
                                   &info,
                                   sizeof(info),
                                   SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES)) {
                    icon = QtWin::fromHICON(info.hIcon);
                    //must destroy icon
                    DestroyIcon(info.hIcon);
                    return icon;
                } else
                    return icon;
#else
                //from doc, said this is OK to parse every time;
                QMimeDatabase mime_database;
                QList<QMimeType> mime_types = mime_database.mimeTypesForFileName(item->objectPath);
                for (int i = 0; i < mime_types.count() && icon.isNull(); i++)
                    icon = QIcon::fromTheme(mime_types[i].iconName());
                if (icon.isNull())
                    return iconProvider.icon(QFileIconProvider::File);
                else
                    return icon;
#endif // !Q_OS_WIN



            }
            case S3DirectoryType:
            case S3ParentDirectoryType:
                return iconProvider.icon(QFileIconProvider::Folder);
            case S3BucketType:
                return iconProvider.icon(QFileIconProvider::Drive);
            }
        } else {
            return QVariant();
        }
    } else if (role == Qt::DisplayRole) {
        //return different data for different column
        return item->data.value(index.column());
    } else if (role == Qt::SizeHintRole) {
        //return QSize(40,40);
        return QVariant();
    }
    return QVariant();
}

int S3TreeModel::rowCount(const QModelIndex &parent) const
{
    return m_currentData.size();
}

int S3TreeModel::columnCount(const QModelIndex &parent) const
{
    //only display the "key, mtime, owner, size
    return m_titles.size();
}

QVariant S3TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        //for bucket, the second column should be ctime, so, change this to ctime.
        if (section == 1 && m_currentPath == "/") {
            return QVariant("Ctime");
        }
        return m_titles.value(section);
    }

    return QVariant();
}

QModelIndex S3TreeModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}



Qt::ItemFlags S3TreeModel::flags(const QModelIndex &index) const
{
    return (QAbstractItemModel::flags(index)) & ~ Qt::ItemIsEditable;
}



void S3TreeModel::deleteObject(const QModelIndex &index)
{
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
    auto deleteAction = m_s3client->DeleteObject(item->bucketName, item->objectPath);
    qDebug() << "Delete Action:";
    qDebug() << "BucketName: " << item->bucketName;
    qDebug() << "ObjectPath: " << item->objectPath;

    connect(deleteAction, &DeleteObjectAction::DeleteObjectFinished, this, [ = ](bool success,
    s3error err) {
        if (success) {
            //refresh this s3model
            qDebug() << "delete " << item->objectPath << "success";
            refresh();
        } else {
            qDebug() << "delete " << item->objectPath << "failed";
            //show the messagebox;
        }
        //deleteAction->deleteLater();
        emit this->cmdFinished(success, err);
    });
}

void S3TreeModel::deletePrefix(const QModelIndex &index)
{
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
    QChar separator('/');
    QString bucketName = item->bucketName;
    QString prefix = item->objectPath;
    if (!prefix.endsWith(separator))
        prefix.append(separator);

    m_deleteObjects.clear();
    ListObjectAction *loAction = m_s3client->ListObjects(bucketName, "", prefix, "");
    connect(loAction, &ListObjectAction::ListObjectInfo, this, [ = ](s3object object,
    QString bucketName) {
        m_deleteObjects << AwsString2QString(object.GetKey());
    });
    connect(loAction, &ListObjectAction::ListPrefixInfo, this, [ = ](s3prefix prefix,
    QString bucketName) {
        m_deleteObjects << AwsString2QString(prefix.GetPrefix());
    });
    connect(loAction, &ListObjectAction::ListObjectFinished, this, [ = ](bool success, s3error error,
    bool truncated, QString nextMarker) {
        auto action = qobject_cast<ListObjectAction *>(sender());
        action->deleteLater();

        m_deleteFinishedCnt = 0;
        for (auto &object : m_deleteObjects) {
            auto deleteAction = m_s3client->DeleteObject(bucketName, object);
            connect(deleteAction, &DeleteObjectAction::DeleteObjectFinished, this, [ = ](bool success,
            s3error err) {
                if (++m_deleteFinishedCnt == m_deleteObjects.size())
                    refresh();
            });
        }
    });
}

QString S3TreeModel::getRootPath()
{
    return m_currentPath;
}

QString S3TreeModel::getRootBucket()
{
    QStringList parts = m_currentPath.split("/");
    if (parts.length() >= 2)
        return parts[1];
    return QString(); /*empty string*/
}

/* /a/b/c/  => b/c/ */
QString S3TreeModel::getCurrentPrefix()
{
    QString bucketName = getRootBucket();
    if (bucketName.isEmpty())
        return QString();
    int l = QString("/" + bucketName + "/").length();
    return m_currentPath.mid(l);
}

QDebug operator<<(QDebug dbg, const SimpleItem *item)
{
    dbg.nospace() << "Bucket:" << item->bucketName << " "
                  << "Path:"   << item->objectPath << " ";
    return dbg;
}
