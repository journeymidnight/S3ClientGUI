#include "qtaskmodel.h"
#include <QDebug>

QString directionStringMapper(TaskDirection t) {
    QString s[] = {
        "==>",
        "<=="
    };
    return s[static_cast<int>(t)];
}


QString taskStatusStringMapper(TaskStatus s) {
    QString status[] = {
        "Queueing",
        "Running",
        "Suspended",
        "Failed",
        "SuccessCompleted"
    };
    return status[static_cast<int>(s)];
}

QTaskModel::QTaskModel(QObject *parent):QAbstractTableModel(parent) {
    //one thread reserved for cmd line tools such as list/delete/
    int availableSlot = QThreadPool::globalInstance()->maxThreadCount() - 1;
    m_scheduler = new QTaskScheduler(this, QThreadPool::globalInstance(), availableSlot);
    m_scheduler->start();
}

int QTaskModel::rowCount(const QModelIndex &parent) const {
    return m_tasks.length();
}

int QTaskModel::columnCount(const QModelIndex &parent) const{
    return DISPLAY_TASK_COLUNM;
}


//works like QTaskModel::index(row, column, parent)
QModelIndex QTaskModel::indexOfTask(int column, const QSharedPointer<TransferTask> t) const {
    int row = 0;
    bool found = false;
    for(auto i:m_tasks) {
        if(i == t) {
            found = true;
            break;
        }
        row ++;
    }
    if (found)
        //return createIndex(row, column, (void*)NULL);
        return createIndex(row, column, (void*)&m_tasks[row]);
    else
        return QModelIndex();
    /*
    const void * p = &m_tasks[row];
    QSharedPointer<TransferTask> x = *(QSharedPointer<TransferTask>*)p;
    */
}

QVariant QTaskModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int column = index.column();
    const QSharedPointer<TransferTask> task = m_tasks.at(row);
    if (role == Qt::DisplayRole)  {
        switch (index.column()) {
        case 0:
            return task->localFileName;
        case 1:
            return directionStringMapper(task->transferType);
        case 2:
            return task->remoteFileName;
        case PROGRESS_COLUMN:
            return task->progress;
        case SIZE_COLUMN:
            return task->size;
        case STATUS_COLUMN:
            return taskStatusStringMapper(task->status);
        }
        // use qt::userrole to filter model
    }  if (role == Qt::UserRole && column == 0) {
        return taskStatusStringMapper(task->status);
    }  else {
        return QVariant();

    }

}

Qt::ItemFlags QTaskModel::flags(const QModelIndex &index) const {
    return (QAbstractItemModel::flags(index)) & ~ Qt::ItemIsEditable;
}

void QTaskModel::addTask(QSharedPointer<TransferTask> t) {

    int lastRow = m_tasks.length();
    beginInsertRows(QModelIndex(),lastRow, lastRow);
    m_tasks.push_back(t);
    endInsertRows();

    m_scheduler->addToPendingPool(t);


    //CALLBACK FUNCTION: udpateStatus
    connect(t->pInstance, &ObjectHandlerInterface::updateStatus, this, [this, t](TransferStatus status) {
        //this make sure the slot run in ui thread;
        qDebug() << "inside ui thread" << QThread::currentThread();
        TaskStatus result;
        switch (status) {
            case TransferStatus::FAILED:
                result = TaskStatus::Failed;
                break;
            case TransferStatus::CANCELED:
                result = TaskStatus::Suspended;
                break;
            case TransferStatus::COMPLETED:
                result = TaskStatus::SuccessCompleted;
                break;
            case TransferStatus::IN_PROGRESS:
                result = TaskStatus::Running;
                break;
            case TransferStatus::EXACT_OBJECT_ALREADY_EXISTS:
                result = TaskStatus::Failed;
                break;
            case TransferStatus::ABORTED:
                result = TaskStatus::Failed;
                break;
                /*ingnore NOT_STARTED*/
            case TransferStatus::NOT_STARTED:
            ;
        }
        qDebug() << "status :" << taskStatusStringMapper(result);
        t->status = result;
        const QModelIndex &statusIndex = this->indexOfTask(STATUS_COLUMN, t);
        emit dataChanged(statusIndex, statusIndex);

    });


    //CALLBACK FUNCTION: updateProgress
    connect(t->pInstance, &ObjectHandlerInterface::updateProgress, this, [this, t](uint64_t transfered, uint64_t total){
        //update size first if not set
        QString totalString = QString("%1").arg(total);
        if(t->size.isEmpty() || t->size != totalString) {
            t->size = totalString;
            const QModelIndex &sizeIndex = this->indexOfTask(SIZE_COLUMN, t);
            emit dataChanged(sizeIndex, sizeIndex);
        }


        t->progress = (transfered * 100) / total;
        if (t->progress % 5 != 0) {
            return;
        }

        const QModelIndex &progressIndex = this->indexOfTask(PROGRESS_COLUMN, t);
        emit dataChanged(progressIndex, progressIndex);


        emit TaskUpdateProgress(t);
    });

}


QVariant QTaskModel::headerData(int section, Qt::Orientation orientation,
                        int role) const {
    QString titles[DISPLAY_TASK_COLUNM] = {
        "localfile",
        "direction",
        "remotefilename",
        "progress",
        "size",
        "status"
    }
        ;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return titles[section];
    }
    return QVariant();

}

const QSharedPointer<TransferTask> QTaskModel::taskAtRow(const QModelIndex &index) {
    int row = index.row();
    const QSharedPointer<TransferTask> t = m_tasks.at(row);
    return t;
}

int QTaskModel::stopAll() {
    int jobs = 0;
    for(auto task : m_tasks) {
        /*
        if (!task.isNull() && task->pInstance != NULL) {
            qDebug() << task;
            qDebug() << task->pInstance;
            task->pInstance->stop();
            jobs ++;
        }
        */
        if(task->pInstance)
            task->pInstance->stop();
    }
    m_scheduler->stopme();
    qDebug() << "QTaskModel is closed";
    return jobs;
}

int QTaskModel::runningJobs() {
    return m_scheduler->runningJobs();
}

QTaskModel::~QTaskModel() { 
    m_scheduler->stopme();
    m_scheduler->wait();
}




QTaskScheduler::QTaskScheduler(QObject *parent, QThreadPool * instance, int slot):QThread(parent), m_poolInstance(instance), avalibleSlot(slot){
    m_runningJobs = 0;
    stop = false;
    this->start();
}

void QTaskScheduler::stopme() {
    stop = true;
    slotAvailiableLock.wakeAll();
    queueEmptyLock.wakeAll();
    //this should be stopped.
}



bool QTaskScheduler::hasSlotToRun() {
    qDebug() << "avalibleSlot is" << avalibleSlot;
    qDebug() << "empty thread is" << m_poolInstance->maxThreadCount() - m_poolInstance->activeThreadCount();
    if(avalibleSlot > 0 && (m_poolInstance->maxThreadCount() - m_poolInstance->activeThreadCount()) > 0) {
        return true;
    } else {
        return false;
    }
}

void QTaskScheduler::run() {
    while(!stop) {
        qDebug() << "scheduler";
        mutex.lock();
        while (hasSlotToRun()==false) {
            qDebug() << "scheduler1";
            slotAvailiableLock.wait(&mutex);
            if (stop == true) {
                mutex.unlock();
                return;
            }
        }
        //the task could be deleted when still in queue;
        while (m_pendingList.size() == 0 ){
            qDebug() << "scheduler2";
            queueEmptyLock.wait(&mutex);
            if (stop == true) {
                qDebug() << "Get Queue empty lock ,will return";
                mutex.unlock();
                return;
            }
        }

        qDebug() << "scheduler3";
        QWeakPointer<TransferTask> wt =  m_pendingList.dequeue();
        QSharedPointer<TransferTask> st = wt.lock();
        if (!st.isNull()&&!stop) {
            qDebug() << "scheduler4";
            //update the status

            st->pInstance->start();
            //THREAD POOL release a thread or more
            m_runningJobs ++;
            this->avalibleSlot --;
        }
        mutex.unlock();
    }
}

int QTaskScheduler::addToPendingPool(QSharedPointer<TransferTask> t) {
    //UI thread
    QWeakPointer<TransferTask> wt = t.toWeakRef();
    mutex.lock();
    m_pendingList.enqueue(wt);
    if (m_pendingList.size() > 0 )
        queueEmptyLock.wakeAll();

    //when task is finished, put slot back and emit the taskfinished signal
    connect(t->pInstance, &ObjectHandlerInterface::finished, this, [=](bool success, s3error err){
        if (success) {
            qDebug() << "UI thread:" << QThread::currentThread() << "result:" << success;
        } else {
            qDebug() << "UI thread:" << QThread::currentThread() << "error why?" << err.GetExceptionName().c_str();
        }
        mutex.lock();
        //THREAD POOL user a new thread or more
        this->avalibleSlot ++;
        m_runningJobs --;
        slotAvailiableLock.wakeAll();
        mutex.unlock();

        delete t->pInstance;
        t->pInstance = NULL;

        QTaskModel * task_model = qobject_cast<QTaskModel*>(parent());
        qDebug() << "emit Taskfinished signal";
        emit task_model->TaskFinished(t);
    });
    mutex.unlock();
	return 0;
}

int QTaskScheduler::runningJobs()
{
    int temp = 0;
    mutex.lock();
    temp = m_runningJobs;
    mutex.unlock();
    return temp;
}
