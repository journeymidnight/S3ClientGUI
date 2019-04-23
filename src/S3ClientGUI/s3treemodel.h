#ifndef S3TREEMODEL_H
#define S3TREEMODEL_H
#include <QFileIconProvider>
#include <QAbstractItemModel>
#include "../s3util/qs3client.h"
#include "s3item.h"
#include <qtimer.h>

using namespace qlibs3;

struct SimpleItem
{
    explicit SimpleItem(const QList<QVariant> &data, S3Type type, QString bucketName, QString objectPath):
        type(type),
        data(data),
        bucketName(bucketName),
        objectPath(objectPath)
    {
    }

    S3Type type;
    QList<QVariant> data; /* For Display */
    QString bucketName;
    QString objectPath;
    /*bucketName, Prefix, data*/
};
//just work like cout, and SimpleItem is a struct, no need for keyword *friend*
QDebug operator<<(QDebug dbg, const SimpleItem *item);



class S3TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    S3TreeModel(QS3Client * s3client, QObject * parent=0);
    ~S3TreeModel();
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QString getRootPath();

     // get "bucketName"  or ""
    QString getRootBucket();
    QString getCurrentPrefix();
	inline void setS3Client(QS3Client *s3client) { m_s3client = s3client; }

signals:
    void rootPathChanged(const QString & newPath);
    void updateInfo(const QString &status);
    void cmdFinished(bool, s3error);
	void currentViewIsBucket(bool);

public slots:
    void setRootPath(const QString &path);
    void setRootIndex(const QModelIndex & index);
    void refresh();
    void deleteObject(const QModelIndex & index);
	void deletePrefix(const QModelIndex & index);

private slots://could use lamda function to replace
    void listBucketInfo(s3bucket bucket);
    void listObjectInfo(s3object object, QString bucketName);
    void listPrefixInfo(s3prefix prefix, QString bucketName);

    void listBucketFinishd(bool success, s3error error);
    void listObjectFinished(bool,s3error,bool,QString);

    //void DeleteObjectFinished(bool success, s3error error); using lamda function

private:
	QString toValidPath(QString path);

private:
	SimpleItem *m_tempData;
    QS3Client *m_s3client;
    QList<SimpleItem*> m_currentData;
    bool m_truncated;
    QString m_currentPrefix;
    QList<QVariant> m_titles;
    QFileIconProvider iconProvider;
    QString m_currentPath;
    CommandAction *m_currentCommand;
	const QString dotdot = QString("..");
	QStringList m_deleteObjects;
	int m_deleteFinishedCnt;

	QTimer *timer = new QTimer(this);
};

#endif // S3TREEMODEL_H
