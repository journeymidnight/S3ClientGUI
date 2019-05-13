#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfilesystemview.h"
#include "qtaskmodel.h"
#include "driveselectwidget.h"
#include "qs3config.h"
#include "editaccountdialog.h"

#include <QMenu>
#include <QMenuBar>
#include <QDir>
#include <QSharedPointer>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QEvent>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_s3client(nullptr)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::rebuildS3Client()
{
    QS3Config::S3Config s3config = QS3Config::Instance()->getS3Config();
    if (m_s3client)
        m_s3client->deleteLater();

    m_s3client = new QS3Client(this,
                               s3config.endpoint,
                               s3config.schema,
                               s3config.accessKey,
                               s3config.secretKey);

    m_s3client->Connect();
}

void MainWindow::init()
{
    createMenus();

    S3API_INIT();

    rebuildS3Client();

    m_s3model = new S3TreeModel(m_s3client, this);
    connect(m_s3model, SIGNAL(rootPathChanged(QString)), ui->S3PathEdit,
            SLOT(setText(const QString &)));
    connect(m_s3model, SIGNAL(currentViewIsBucket(bool)), this, SLOT(on_enableBucketActions(bool)));
    //connect(m_s3model, SIGNAL(updateInfo(QString)), ui->S3Info, SLOT(setText(QString)));

    m_s3model->setRootPath("/");

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_s3model);

    ui->splitter->setStretchFactor(0, 4);
    ui->splitter->setStretchFactor(1, 3);

    ui->treeViewS3->setModel(proxyModel);
    ui->treeViewS3->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeViewS3->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeViewS3->setItemsExpandable(false);
    ui->treeViewS3->setRootIsDecorated(false);
    ui->treeViewS3->setExpandsOnDoubleClick(false);
    ui->treeViewS3->setSortingEnabled(true);
    ui->treeViewS3->header()->setSectionsMovable(false);
    ui->treeViewS3->header()->setStretchLastSection(true);
    ui->treeViewS3->setColumnWidth(1, 107);
    ui->treeViewS3->sortByColumn(1);

    ui->treeViewS3->installEventFilter(this);

    m_fsview = new QFilesystemView(this);
    //Add a new View to layout
    ui->verticalLayout_localFSView->insertWidget(1, m_fsview);

    DriveSelectWidget *driveSelect = new DriveSelectWidget(this);
    driveSelect->setReadOnly(true);
    connect(driveSelect, &DriveSelectWidget::driveChanged, m_fsview, &QFilesystemView::changeToDrive);
    ui->horizontalLayout->insertWidget(1, driveSelect);

    ui->S3PathEdit->setReadOnly(true);


    //s3 view
    //model of ui->treeViewS3 is proxyModel, must use mapToSource() to convert to m_s3model
    connect(ui->treeViewS3, &QTreeView::doubleClicked, this, [ = ](QModelIndex proxyIndex) {
        if (!proxyIndex.isValid())
            return;
        QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
                                proxyIndex);
        m_s3model->setRootIndex(index);
    });

    //init BucketAction Area
    //ui->horizontalLayout_BucketAction->addStretch();

    ui->toolButton_bucketCreate->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_bucketCreate->setFocusPolicy(Qt::NoFocus);
    ui->toolButton_bucketCreate->setIcon(QIcon(":/images/bucket_Create.png"));
    ui->toolButton_bucketCreate->setIconSize(QSize(24, 24));
    ui->toolButton_bucketCreate->setText(tr("New bucket"));
    connect(ui->toolButton_bucketCreate, SIGNAL(clicked()), this, SLOT(on_bucketCreate()));

    ui->toolButton_bucketDelete->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_bucketDelete->setFocusPolicy(Qt::NoFocus);
    ui->toolButton_bucketDelete->setIcon(QIcon(":/images/bucket_Delete.png"));
    ui->toolButton_bucketDelete->setIconSize(QSize(24, 24));
    ui->toolButton_bucketDelete->setText(tr("Delete bucket"));
    connect(ui->toolButton_bucketDelete, SIGNAL(clicked()), this, SLOT(on_bucketDelete()));

    ui->toolButton_bucketRefresh->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_bucketRefresh->setFocusPolicy(Qt::NoFocus);
    ui->toolButton_bucketRefresh->setIcon(QIcon(":/images/bucket_Refresh.png"));
    ui->toolButton_bucketRefresh->setIconSize(QSize(24, 24));
    ui->toolButton_bucketRefresh->setText(tr("Refresh"));
    connect(ui->toolButton_bucketRefresh, SIGNAL(clicked()), this, SLOT(on_bucketRefresh()));

    ui->toolButton_mkdir->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->toolButton_mkdir->setFocusPolicy(Qt::NoFocus);
    ui->toolButton_mkdir->setIcon(QIcon(":/images/bucket_Create.png"));
    ui->toolButton_mkdir->setIconSize(QSize(24, 24));
    ui->toolButton_mkdir->setText(tr("New Folder"));
    connect(ui->toolButton_mkdir, SIGNAL(clicked()), this, SLOT(on_mkdir()));



    //fs view
    //connect(m_fsview, SIGNAL(updateInfo(QString)), ui->localInfo, SLOT(setText(QString)));
    connect(m_fsview, SIGNAL(rootPathChanged(QString)), driveSelect, SLOT(setText(QString)));
    //connect(ui->localUpButton, SIGNAL(clicked()), m_fsview, SLOT(upToParent()));
    m_fsview->refreshSignals();

    //to receive log
    connect(m_s3client, SIGNAL(logReceived(QString)), ui->logEdit, SLOT(append(QString)));

    //actions
    ui->treeViewS3->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeViewS3, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(on_S3ContextMenuRequest(QPoint)));


    m_fsview->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_fsview, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(on_LocalContextMenuRequest(QPoint)));

    qDebug() << "main thread" << QThread::currentThread();


    m_transferTabWidget = new TransferTabWidget(this);
    ui->splitter_2->insertWidget(2, m_transferTabWidget);


    connect(m_transferTabWidget, SIGNAL(TaskFinished(QSharedPointer<TransferTask>)),
            this, SLOT(on_taskFinished(QSharedPointer<TransferTask>)));

    ui->logEdit->hide();

    quitDialog = 0;
    connect(m_s3model, SIGNAL(cmdFinished(bool, s3error)), this, SLOT(on_cmdFinished(bool, s3error)));
}

void MainWindow::createMenus()
{
    exitAction = new QAction(tr("E&xit"), this);
    /*
     * For mac, if we do not define the menu's Exit action, 'cmd+Q' action will
     * call MainWindow::closeEvent twice.
     * https://stackoverflow.com/questions/23274983/qt-mainwindow-closeevent-mac-cmdq
     * And Most Qt examples have this similar exitAction<=>close() thing
     */
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAction);
}

void MainWindow::on_bucketCreate()
{
    bool ok;
    QString bucketName = QInputDialog::getText(this,
                                               tr("Create New Bucket"),
                                               tr("Bucket name:"),
                                               QLineEdit::Normal,
                                               QString(),
                                               &ok);

    if (ok && !bucketName.isEmpty()) {
        CreateBucketAction *action = m_s3client->CreateBucket(bucketName);
        connect(action, &CreateBucketAction::CreateBucketFinished, this, [ = ](bool success, s3error err) {
            if (success)
                m_s3model->setRootPath(QString('/'));
            else {
                Aws::String exceptionName = err.GetExceptionName();
                Aws::String message = err.GetMessage();
                if (message.length() == 0)
                    message = exceptionName;
                QMessageBox::critical(this,
                                      AwsString2QString(exceptionName),
                                      AwsString2QString(message),
                                      QMessageBox::Yes);
            }
        });
    }
}

void MainWindow::on_bucketDelete()
{
    QModelIndex proxyIndex = ui->treeViewS3->currentIndex();
    if (!proxyIndex.isValid())
        return;
    QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
                            proxyIndex);
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
    if (QMessageBox::question(this,
                              tr("Confirm Bucket Deletion"),
                              tr("<html><p>The following bucket will be permanently deleted:</p><p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<strong>%1</strong></p></html>").arg(
                                  item->objectPath),
                              QMessageBox::Yes | QMessageBox::Cancel,
                              QMessageBox::Cancel) == QMessageBox::Yes) {
        DeleteBucketAction *action = m_s3client->DeleteBucket(item->objectPath);
        connect(action, &DeleteBucketAction::DeleteBucketFinished, this, [ = ](bool success, s3error err) {
            if (success)
                m_s3model->setRootPath(QString('/'));
            else {
                Aws::String exceptionName = err.GetExceptionName();
                Aws::String message = err.GetMessage();
                if (message.length() == 0)
                    message = exceptionName;
                QMessageBox::critical(this,
                                      AwsString2QString(exceptionName),
                                      AwsString2QString(message),
                                      QMessageBox::Yes);
            }
        });
    }
}

void MainWindow::on_bucketRefresh()
{
    //m_s3model->setRootPath(QString('/'));
    m_s3model->refresh();
}

void MainWindow::on_mkdir()
{
    bool ok;
    QString dirName = QInputDialog::getText(this,
                                            tr("Create New Folder"),
                                            tr("Folder name:"),
                                            QLineEdit::Normal,
                                            QString(),
                                            &ok);

    if (!dirName.endsWith('/'))
        dirName.append('/');
    if (ok && !dirName.isEmpty()) {
        QString bucketName = m_s3model->getRootBucket();
        QString currentPrefix = m_s3model->getCurrentPrefix();
        PutObjectAction *action = m_s3client->PutObject(bucketName, currentPrefix + dirName);
        connect(action, &PutObjectAction::PutObjectFinished, this, [ = ](bool success, s3error err) {
            if (success)
                m_s3model->setRootPath(m_s3model->getRootPath());
            else
                QMessageBox::critical(this,
                                      AwsString2QString(err.GetExceptionName()),
                                      AwsString2QString(err.GetMessage()),
                                      QMessageBox::Yes);
        });
    }
}

void MainWindow::on_enableBucketActions(bool enabled)
{
    ui->toolButton_bucketCreate->setEnabled(enabled);
    ui->toolButton_bucketDelete->setEnabled(enabled);
    //ui->toolButton_bucketRefresh->setEnabled(enabled);
    ui->toolButton_mkdir->setEnabled(!enabled);
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
        int pos = p.lastIndexOf('/', -2);
        if (pos != 1) {
            parentPath = p.mid(0, pos + 1);
        }
    }
    qDebug() << "Jump to Parent Path" << parentPath;

    if (parentPath != p) {
        m_s3model->setRootPath(parentPath);
    }
}


void MainWindow::on_S3ContextMenuRequest(const QPoint &point)
{
    QModelIndex proxyIndex = ui->treeViewS3->indexAt(point);
    if (proxyIndex.isValid() == false) {
        return;
    }
    QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
                            proxyIndex);
    //temperate menu;
    QMenu menu;

    //QAction *addAction(const QString &text, const QObject *receiver, const char* member, const QKeySequence &shortcut = 0);
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
    if (item->type == S3FileType )  {
        menu.addAction("Download", this, SLOT(on_download()));
        menu.addAction("Delete", this, SLOT(on_delete()));
    } else if (item->type == S3ParentDirectoryType)
        menu.addAction("Open", this, SLOT(on_open()));
    else if (item->type == S3DirectoryType) {
        menu.addAction("Download", this, SLOT(on_downloadDir()));
        menu.addAction("Open", this, SLOT(on_open()));
        menu.addAction("Delete", this, SLOT(on_delete()));
    }

    menu.exec(ui->treeViewS3->mapToGlobal(point));
}


void MainWindow::on_LocalContextMenuRequest(const QPoint &point)
{
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
    } else if (info.isDir() && index.data().toString() != ".." && !m_s3model->getRootBucket().isEmpty()) {
        menu.addAction("Upload", this, SLOT(on_uploadDir()));
        menu.exec(m_fsview->mapToGlobal(point));
    } else {
        return;
    }
}

QFileInfoList GetFileList(QString path)
{
    QDir dir(path);
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i != folder_list.size(); i++) {
        QString name = folder_list.at(i).absoluteFilePath();
        QFileInfoList child_file_list = GetFileList(name);
        if (child_file_list.empty())
            file_list.append(name);
        file_list.append(child_file_list);
    }
    if (file_list.empty())
        file_list.append(path);
    return file_list;
}

void MainWindow::on_uploadDir()
{
    QFileInfoList fil;
    QFileInfo info = m_fsview->currentFileInfo();
    QString parentDirPath = info.absolutePath() + '/';
    qDebug() << "info=" << qPrintable(info.absolutePath()) << endl;
    QString remoteBucketName = m_s3model->getRootBucket();
    QString remotePrefix = m_s3model->getCurrentPrefix();
    QString KeyName;

    fil = GetFileList(info.absoluteFilePath());
    for (int i = 0; i < fil.size(); i++) {
        qDebug() << "fil=" << fil.at(i) << endl;
    }
    for (int i = 0; i < fil.size(); ++i) {
        QString localFile = fil.at(i).absoluteFilePath();
        QString tmp = localFile;
        tmp.remove(0, parentDirPath.length() - 1);
        KeyName = remotePrefix + tmp;
        qDebug() << "tmp=" << qPrintable(tmp) << endl;
        qDebug() << "KeyName=" << qPrintable(KeyName) << endl;
        //if dir is empty,mkdir!
        if (!fil.at(i).isFile()) {
            tmp = remotePrefix + tmp + '/';
            PutObjectAction *action = m_s3client->PutObject(remoteBucketName, tmp);
            connect(action, &PutObjectAction::PutObjectFinished, this, [=](bool success,
                s3error err) {
                if (success)
                    m_s3model->setRootPath(m_s3model->getRootPath());
                else
                    QMessageBox::critical(this,
                        AwsString2QString(err.GetExceptionName()),
                        AwsString2QString(err.GetMessage()),
                        QMessageBox::Yes);
            });
            continue;
        }
        UploadObjectHandler *pHandler = m_s3client->UploadFile(localFile, remoteBucketName, KeyName, "");

        QSharedPointer<TransferTask> t = QSharedPointer<TransferTask>(new TransferTask);

        t->localFileName = localFile;
        t->remoteFileName = m_s3model->getRootPath() + info.fileName();
        t->size = "unknown";
        t->status = TaskStatus::Queueing;
        t->transferType = TaskDirection::Upload;
        t->pInstance = pHandler;

        m_transferTabWidget->addTask(t);
    }
}

void MainWindow::on_upload()
{
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

    UploadObjectHandler *pHandler = m_s3client->UploadFile(localFile, remoteBucketName, KeyName, "");
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

void MainWindow::on_open()
{
    QModelIndex proxyIndex = ui->treeViewS3->currentIndex();
    if (!proxyIndex.isValid())
        return;
    QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
                            proxyIndex);
    m_s3model->setRootIndex(index);
}

void MainWindow::on_delete()
{
    QModelIndex proxyIndex = ui->treeViewS3->currentIndex();
    if (!proxyIndex.isValid())
        return;
    QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
                            proxyIndex);
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
    if (QMessageBox::question(this,
                              tr("Confirm File Delete"),
                              tr("<html><p>Are you sure you want to delete</p><p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<strong>%1</strong></p></html>").arg(
                                  item->objectPath),
                              QMessageBox::Yes | QMessageBox::Cancel,
                              QMessageBox::Cancel) == QMessageBox::Yes) {
        if (item->type == S3FileType)
            m_s3model->deleteObject(index);
        else if (item->type == S3DirectoryType)
            m_s3model->deletePrefix(index);
    }
}

void MainWindow::on_download()
{
    QModelIndex proxyIndex = ui->treeViewS3->currentIndex();
    if (!proxyIndex.isValid())
        return;
    QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
                            proxyIndex);
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());

    //the download file name: downloadPath + S3DisplayName
    //system path
    //define a new function
    QChar seperator = '/';
    QString slash;

    //if in root path
    if (m_fsview->currentPath().endsWith(seperator) == true)
        slash = "";
    else
        slash = seperator;

    QString downloadFile = m_fsview->currentPath() + slash + item->data[0].toString();
    downloadFile = QDir::fromNativeSeparators(downloadFile);

    //check permission
    QFileInfo path(m_fsview->currentPath());
    //must be a dir
    if (path.isDir() && path.isWritable() == false) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("\"%1\" is not writable").arg(m_fsview->currentPath()),
                             QMessageBox::Yes);
        return;
    }

    DownloadObjectHandler *pHandler = m_s3client->DownloadFile(item->bucketName, item->objectPath,
                                                               downloadFile);

    qDebug() << "BucketName: " << item->bucketName;
    qDebug() << "ObjectPath: " << item->objectPath;
    qDebug() << "DownloadFile" << downloadFile;



    QSharedPointer<TransferTask> t = QSharedPointer<TransferTask>(new TransferTask);

    t->localFileName = downloadFile;
    t->remoteFileName = m_s3model->getRootPath() + item->objectPath;
    //t->remoteFileName = item->bucketName + seperator + item->objectPath;
    t->status = TaskStatus::Queueing;
    t->transferType = TaskDirection::Download;
    t->pInstance = pHandler;
    t->progress = 0;

    m_transferTabWidget->addTask(t);
    
    qDebug() << "inside UI thread:" << QThread::currentThread();

    return;
}

void MainWindow::on_downloadDir()
{
    QModelIndex proxyIndex = ui->treeViewS3->currentIndex();
    if (!proxyIndex.isValid())
        return;
    QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
        proxyIndex);
    SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
    QString bucketName = item->bucketName;
    QString prefix = item->objectPath;
    QChar seperator = '/';
    QString slash;
    //if in root path
    if (m_fsview->currentPath().endsWith(seperator) == true)
        slash = "";
    else
        slash = seperator;
    //check permission
    QFileInfo path(m_fsview->currentPath());
    //must be a dir
    if (path.isDir() && path.isWritable() == false) {
        QMessageBox::warning(this,
            windowTitle(),
            tr("\"%1\" is not writable").arg(m_fsview->currentPath()),
            QMessageBox::Yes);
        return;
    }

    if (!prefix.endsWith(seperator))
        prefix.append(seperator);
    ListObjectAction *loAction = m_s3client->ListObjects(bucketName, "", prefix, "");
    connect(loAction, &ListObjectAction::ListObjectInfo, this, [=](s3object object,
        QString bucketName) {
        file_list << AwsString2QString(object.GetKey());
    });
    connect(loAction, &ListObjectAction::ListPrefixInfo, this, [=](s3prefix prefix,
        QString bucketName) {
        file_list << AwsString2QString(prefix.GetPrefix());
    });
    connect(loAction, &ListObjectAction::ListObjectFinished, this, [=](bool success, s3error error,
        bool truncated, QString nextMarker) {
        for (int i = 0; i < file_list.size(); ++i) {
            qDebug() << "fileName" << m_fsview->currentPath() << endl;
            qDebug() << "bucket=" << bucketName << endl;
            qDebug() << "keyName=" << file_list.at(i) << endl;
            QString tmpKeyName = file_list.at(i);
            QString currentPrefix = m_s3model->getCurrentPrefix();
            QString localKey = tmpKeyName.remove(0, currentPrefix.length());
            QString downloadFile = m_fsview->currentPath() + slash + localKey;
            //for mkdir
            QString tmp = downloadFile;
            QStringList strl = downloadFile.split('/');
            tmp.chop(strl.at(strl.size() - 1).length());
            qDebug() << "parent Dir=" << tmp << endl;
            QDir dir(tmp);
            if (!dir.exists()) {
                dir.mkpath(tmp);
            }
            if (downloadFile.endsWith("/"))
                continue;
            qDebug() << "tmp=" << tmp << endl;
            qDebug() << "downloadFile=" << downloadFile << endl;
            DownloadObjectHandler *pHandler = m_s3client->DownloadFile(bucketName, file_list.at(i),
                downloadFile);

            QSharedPointer<TransferTask> t = QSharedPointer<TransferTask>(new TransferTask);

            t->localFileName = downloadFile;
            t->remoteFileName = m_s3model->getRootPath() + localKey;
            t->status = TaskStatus::Queueing;
            t->transferType = TaskDirection::Download;
            t->pInstance = pHandler;
            t->progress = 0;

            m_transferTabWidget->addTask(t);
        }
        file_list.clear();
    });
}

void MainWindow::on_taskFinished(QSharedPointer<TransferTask> t)
{
    qDebug() << "on_taskFinished happend";

    if (quitDialog != 0) {
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
        if (m_transferTabWidget->runningJobs() > 0) {
            return;
        } else {
            int pos = t->remoteFileName.lastIndexOf('/');
            //if directoy is still the same, refresh the s3treemodel

            if (t->remoteFileName.left(pos + 1) == currentPath) {
                m_s3model->refresh();
            }
        }
    } else if (t->transferType == TaskDirection::Download) {
        if (m_transferTabWidget->runningJobs() > 0) {
            if (t->status == TaskStatus::ObjectAlreadyExists)
                fileExists++;
            else if (t->status != TaskStatus::SuccessCompleted)
                fileFailed++;
        } else {
            if (fileExists > 0) {
                QMessageBox::warning(this,
                    tr("Download"),
                    tr("Some File Already Exists"));
                fileExists = 0;
            }
            if (fileFailed > 0) {
                QMessageBox::warning(this,
                    tr("Download"),
                    tr("Some File Download Failed"));
                fileFailed = 0;
            }
            if (t->status == TaskStatus::ObjectAlreadyExists) {
                QMessageBox::warning(this,
                    tr("Download"),
                    tr("File Already Exists"));
            } else if (t->status != TaskStatus::SuccessCompleted) {
                QMessageBox::warning(this,
                    tr("Download"),
                    tr("File Download Failed"));
            }
        }
    }
}

void MainWindow::on_cmdFinished(bool success, s3error error)
{
    /*
    if(quitDialog != 0) {
        if (QThreadPool::globalInstance()->activeThreadCount() == 0) {
            quitDialog->close();
        }
    }
    */
    if (!success) {
        Aws::String exceptionName = error.GetExceptionName();
        Aws::String message = error.GetMessage();
        Aws::S3::S3Errors errtype = error.GetErrorType();
        if (message.length() == 0)
            message = exceptionName;
        QMessageBox::critical(this,
                              AwsString2QString(exceptionName),
                              AwsString2QString(message),
                              QMessageBox::Yes);

        if (errtype == Aws::S3::S3Errors::INVALID_ACCESS_KEY_ID ||
                errtype == Aws::S3::S3Errors::ACCESS_DENIED ||
                errtype == Aws::S3::S3Errors::SIGNATURE_DOES_NOT_MATCH)
            on_actionAccount_triggered();
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
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
    event->accept();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->treeViewS3) {
        QModelIndex proxyIndex = ui->treeViewS3->currentIndex();
        if (!proxyIndex.isValid())
            return false;
        QModelIndex index = static_cast<const QSortFilterProxyModel *>(proxyIndex.model())->mapToSource(
                                proxyIndex);

        if (event->type() == QEvent::KeyPress) {
            // if Key_Enter, enter child object
            if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Enter ||
                    static_cast<QKeyEvent *>(event)->key() == Qt::Key_Return) {
                m_s3model->setRootIndex(index);
                return true;
            }
            // if Key_Backspace, back to parent object or bucket
            else if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Backspace) {
                SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
                if (item->type == S3BucketType)
                    return true;
                else {
                    QModelIndex firstIndex = m_s3model->index(0, 0, index.parent());
                    m_s3model->setRootIndex(firstIndex);
                    return true;
                }
            }
            // if Key_Delete, delete object or bucket
            else if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Delete) {
                SimpleItem *item = static_cast<SimpleItem *>(index.internalPointer());
                if (item->type == S3BucketType) {
                    on_bucketDelete();
                    return true;
                } else if (item->type != S3ParentDirectoryType) {
                    on_delete();
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(object, event);
}
void MainWindow::on_actionAccount_triggered()
{
    EditAccountDialog *eaDlg = new EditAccountDialog(this);
    if (eaDlg->exec() == QDialog::Accepted) {
        rebuildS3Client();
        m_s3model->setS3Client(m_s3client);
        m_s3model->setRootPath("/");
    }
}
