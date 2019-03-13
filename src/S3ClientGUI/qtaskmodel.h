#ifndef QTASKMODEL_H
#define QTASKMODEL_H
#include <QAbstractItemModel>
#include <QSharedPointer>
#include "../s3util/qs3client.h"
#include <QWeakPointer>
#include <QQueue>
#include <QWaitCondition>
#include <QDebug>


using namespace qlibs3;

enum class TaskDirection {
    Upload,
    Download,
    N_PROPERTY
};


QString directionStringMapper(TaskDirection t);


enum class TaskStatus {
    Queueing,
    Running,
    Suspended,
    Failed,
	ObjectAlreadyExists,
    SuccessCompleted,
    N_PROPERTY
};

QString taskStatusStringMapper(TaskStatus s);

struct TransferTask {
    QString uuid;
    ObjectHandlerInterface *pInstance;

    //for display
    QString localFileName;
    TaskDirection transferType;
    QString remoteFileName;
    TaskStatus status;
    QString size;
    int progress;
};

#define DISPLAY_TASK_COLUNM 6


#define SIZE_COLUMN 4
#define PROGRESS_COLUMN 3
#define STATUS_COLUMN 5

class QTaskModel;

class QTaskScheduler : public QThread
{
    Q_OBJECT

public:
    QTaskScheduler(QObject *parent, QThreadPool * pool, int slot);
    int addToPendingPool(QSharedPointer<TransferTask> t);
    void run() Q_DECL_OVERRIDE;
    void stopme();
    bool hasSlotToRun();
    int runningJobs();
private:
    bool stop;
    QQueue<QWeakPointer<TransferTask>> m_pendingList;
    QMutex mutex;
    QWaitCondition slotAvailiableLock;
    QWaitCondition queueEmptyLock;
    QThreadPool * m_poolInstance;
    int avalibleSlot;
    int m_runningJobs;
};

class QTaskModel:public QAbstractTableModel
{
    Q_OBJECT
public:
    QTaskModel(QObject *parent=0);
    ~QTaskModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE ;
    void addTask(QSharedPointer<TransferTask> t);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;


    const QSharedPointer<TransferTask> taskAtRow(const QModelIndex &);
    QModelIndex indexOfTask(int column, const QSharedPointer<TransferTask> t) const;
    int stopAll();
    int runningJobs();
signals:
    void TaskUpdateProgress(QSharedPointer<TransferTask> t);
    void TaskFinished(QSharedPointer<TransferTask> t);

private:
    QList<QSharedPointer<TransferTask>> m_tasks;
    QTaskScheduler *m_scheduler;
};



#endif // QTASKMODEL_H
