#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfilesystemview.h"
#include "qtaskmodel.h"
#include "driveselectwidget.h"

#include <QDir>
#include <QSharedPointer>
#include <QMessageBox>
#include <QSortFilterProxyModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::init() {

    S3API_INIT();

    m_s3client = new QS3Client(this,"los-cn-north-1.lecloudapis.com", "http",
                               "Ltiakby8pAAbHMjpUr3L", "qMTe5ibLW49iFDEHNKqspdnJ8pwaawA9GYrBXUYc");

    m_s3model = new S3TreeModel(m_s3client, this);

    m_s3client->Connect();

    m_s3model->setRootPath("/");

	QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
	proxyModel->setSourceModel(m_s3model);

    ui->treeViewS3->setModel(proxyModel);
    ui->treeViewS3->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeViewS3->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeViewS3->setItemsExpandable(false);
    ui->treeViewS3->setRootIsDecorated(false);
    ui->treeViewS3->setExpandsOnDoubleClick(false);
    ui->treeViewS3->setSortingEnabled(true);
	ui->treeViewS3->header()->setSectionsMovable(false);
	ui->treeViewS3->sortByColumn(1);


    m_fsview = new QFilesystemView(this);
    //Add a new View to layout
    ui->verticalLayout->insertWidget(1, m_fsview);

	DriveSelectWidget *driveSelect = new DriveSelectWidget(this);
	driveSelect->setReadOnly(true);
	connect(driveSelect, &DriveSelectWidget::driveChanged, m_fsview, &QFilesystemView::changeToDrive);
	ui->horizontalLayout->insertWidget(1, driveSelect);

	ui->S3PathEdit->setReadOnly(true);


    //s3 view
    connect(m_s3model, SIGNAL(rootPathChanged(QString)),  ui->S3PathEdit, SLOT(setText(const QString &)));
//    connect(ui->treeViewS3, SIGNAL(doubleClicked(QModelIndex)), m_s3model, SLOT(setRootIndex(QModelIndex))); 
	//model of ui->treeViewS3 is proxyModel, must use mapToSource() to convert to m_s3model
	connect(ui->treeViewS3, &QTreeView::doubleClicked, this, [=](QModelIndex proxyIndex) {
		QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(proxyIndex);
		m_s3model->setRootIndex(index);
	});
    //connect(m_s3model, SIGNAL(updateInfo(QString)), ui->S3Info, SLOT(setText(QString)));




    //fs view
    //connect(m_fsview, SIGNAL(updateInfo(QString)), ui->localInfo, SLOT(setText(QString)));
    connect(m_fsview, SIGNAL(rootPathChanged(QString)), driveSelect, SLOT(setText(QString)));
    //connect(ui->localUpButton, SIGNAL(clicked()), m_fsview, SLOT(upToParent()));
    m_fsview->refreshSignals();

    //to receive log
    connect(m_s3client, SIGNAL(logReceived(QString)), ui->logEdit, SLOT(append(QString)));

    //actions
    ui->treeViewS3->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeViewS3, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_S3ContextMenuRequest(QPoint)));


    m_fsview->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_fsview, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_LocalContextMenuRequest(QPoint)));

    qDebug() << "main thread" << QThread::currentThread();


    m_transferTabWidget = new TransferTabWidget(this);
    ui->splitter_2->insertWidget(2, m_transferTabWidget);


    connect(m_transferTabWidget, SIGNAL(TaskFinished(QSharedPointer<TransferTask>)),
            this, SLOT(on_taskFinished(QSharedPointer<TransferTask>)));

    ui->logEdit->hide();

    quitDialog = 0;
    connect(m_s3model, SIGNAL(cmdFinished()), this, SLOT(on_cmdFinished()));
}

/* could be put to S3model */
void MainWindow::on_S3UpButton_clicked()
{
    QString p = m_s3model->getRootPath();
    /*
     *  Path example = /bucketName/dirName/
     *  Path example = /
     *  Path example = /bucketName/
     */
    //find the upper directory;

    QString parentPath = "/";
    if (p != "/") {
        int pos = p.lastIndexOf('/',-2);
        if (pos != 1) {
            parentPath = p.mid(0, pos+1);
        }
    }
    qDebug() << "Jump to Parent Path" << parentPath;

    if (parentPath != p) {
       m_s3model->setRootPath(parentPath);
    }
}


void MainWindow::on_S3ContextMenuRequest(const QPoint & point) {
    QModelIndex index = ui->treeViewS3->indexAt(point);
    if (index.isValid() == false) {
        return;
    }
    //temperate menu;
    QMenu menu;

    //QAction *addAction(const QString &text, const QObject *receiver, const char* member, const QKeySequence &shortcut = 0);
    SimpleItem *item = static_cast<SimpleItem*>(index.internalPointer());
    if(item->type == S3FileType )  {
        menu.addAction("Download", this, SLOT(on_download()));
        menu.addAction("Delete", this, SLOT(on_delete()));
    } else {//S3FileType, S3DiretoryType
        menu.addAction("Open", this, SLOT(on_open()));
    }

    menu.exec(ui->treeViewS3->mapToGlobal(point));

}


void MainWindow::on_LocalContextMenuRequest(const QPoint & point) {
    QModelIndex index = m_fsview->indexAt(point);
    if (index.isValid() == false) {
        return;
    }
    QFileInfo info = m_fsview->currentFileInfo();
    QMenu menu;

    /*
     *  if a single file, and in some bucket, show the "Upload" button
     */
    if (info.isFile() && !m_s3model->getRootBucket().isEmpty()) {
        menu.addAction("Upload", this, SLOT(on_upload()));
        menu.exec(m_fsview->mapToGlobal(point));
    } else {
        return;
    }
}



void MainWindow::on_upload() {
    QFileInfo info = m_fsview->currentFileInfo();
    QString localFile = info.absoluteFilePath();
    //s3 browser current path

    QString remoteBucketName = m_s3model->getRootBucket();
    QString remotePrefix = m_s3model->getCurrentPrefix();
    QString KeyName = remotePrefix + info.fileName();


    qDebug() << "local  file name" << info.fileName();
    qDebug() << "remote bucket name" << remoteBucketName;
    qDebug() << "remote prefix" << remotePrefix;

    if (remoteBucketName == "") {
        //can not upload;
        return;
    }

    UploadObjectHandler * pHandler = m_s3client->UploadFile(localFile,remoteBucketName, KeyName,"");
    //auto pHandler = QSharedPointer<ObjectHandlerInterface>(handler, &QObject::deleteLater);

    QSharedPointer<TransferTask> t = QSharedPointer<TransferTask>(new TransferTask);
    //QT Style shared pointer

    t->localFileName = localFile;
    t->remoteFileName = m_s3model->getRootPath() + info.fileName();
    t->size = "unknown";
    t->status = TaskStatus::Queueing;
    t->transferType = TaskDirection::Upload;
    t->pInstance = pHandler;

    m_transferTabWidget->addTask(t);
}

void MainWindow::on_open() {
    QModelIndex index = ui->treeViewS3->currentIndex();
    m_s3model->setRootIndex(index);
}

void MainWindow::on_delete() {
    QModelIndex index = ui->treeViewS3->currentIndex();
    m_s3model->deleteObject(index);
}

void MainWindow::on_download(){
    QModelIndex index = ui->treeViewS3->currentIndex();
    SimpleItem *item = static_cast<SimpleItem*>(index.internalPointer());

    //the download file name: downloadPath + S3DisplayName
    //system path
    //define a new function
    QChar seperator = QDir::separator();
    QString slash;

    //if in root path
    if (m_fsview->currentPath().endsWith(seperator) == true)
        slash = "";
    else
        slash = seperator;

    QString downloadFile = m_fsview->currentPath() + slash + item->data[0].toString();

    //check permission
    QFileInfo path(m_fsview->currentPath());
    //must be a dir
    if(path.isDir() && path.isWritable() == false) {
        QMessageBox msgBox;
        msgBox.setText(QString("%1 is not writable").arg(m_fsview->currentPath()));
        qDebug() << m_fsview->currentPath() << "is not writable";
        msgBox.exec();
        return;
    }

    DownloadObjectHandler * pHandler = m_s3client->DownloadFile(item->bucketName, item->objectPath, downloadFile);

    qDebug() << "BucketName: " << item->bucketName;
    qDebug() << "ObjectPath: " << item->objectPath;
    qDebug() << "DownloadFile" << downloadFile;



    QSharedPointer<TransferTask> t = QSharedPointer<TransferTask>(new TransferTask);

    t->localFileName = downloadFile;
    t->remoteFileName = item->bucketName + seperator + item->objectPath;
    t->status = TaskStatus::Queueing;
    t->transferType = TaskDirection::Download;
    t->pInstance = pHandler;
    t->progress = 0;

    m_transferTabWidget->addTask(t);

    qDebug() << "inside UI thread:" << QThread::currentThread();

    return;
}


void MainWindow::on_taskFinished(QSharedPointer<TransferTask> t) {

    qDebug() << "on_taskFinished happend";

    if(quitDialog != 0) {
        qDebug() << "WAITING";
        if (m_transferTabWidget->runningJobs() == 0) {
            quitDialog->done(0);
        } else {
            qDebug() << QThreadPool::globalInstance() << "is working";
            qDebug() << "running jobs is" << m_transferTabWidget->runningJobs();
        }
        return;
    }

    if (t->transferType == TaskDirection::Upload) {
        QString currentPath = m_s3model->getRootPath();
        int pos = t->remoteFileName.lastIndexOf('/');
        //if directoy is still the same, refresh the s3treemodel

        if (t->remoteFileName.left(pos + 1) == currentPath) {
            m_s3model->refresh();
        }
    }

}

void MainWindow::on_cmdFinished() {
    /*
    if(quitDialog != 0) {
        if (QThreadPool::globalInstance()->activeThreadCount() == 0) {
            quitDialog->close();
        }
    }
    */
}


void MainWindow::closeEvent(QCloseEvent *event) {
    qDebug() << "ready to bye";
    int jobsToStop = m_transferTabWidget->stopAll();
    //never WAIT for finish
    quitDialog = new QProgressDialog(tr("closing connections"), tr("Abort"), 0, jobsToStop, this);
    while (m_transferTabWidget->runningJobs() > 0) {
        qDebug() << "wait a while, but event loop is still working" << m_transferTabWidget->runningJobs();
        quitDialog->exec();
    }

    //destory API
    S3API_SHUTDOWN();
    qDebug() << "bye~";
}
