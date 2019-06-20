#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../s3util/qs3client.h"
#include "s3treemodel.h"
#include "qfilesystemview.h"
#include <QAction>
#include <QPoint>
#include <transferwidget.h>
#include <QProgressDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();

private slots:
    /* TODO maybe could move to S3-browser-view */
    void on_S3UpButton_clicked();
    void on_S3ContextMenuRequest(const QPoint &);

    void on_LocalContextMenuRequest(const QPoint &);

    void on_download();
    void OnDownloadDir();
    void on_open();
    void on_upload();
    void OnUploadDir();
    void on_delete();
    void addTask(QString bucketName, QString localFilePath, QString remoteFilePath,
        QString key, bool taskStatus);

    void on_taskFinished(QSharedPointer<TransferTask> t);
    void on_cmdFinished(bool, s3error);

    void on_bucketCreate();
    void on_bucketDelete();
    void on_bucketRefresh();
    void on_mkdir();
    void on_enableBucketActions(bool);

    void on_actionAccount_triggered();

private:
    void rebuildS3Client();
    void createMenus();
protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;


private:
    Ui::MainWindow *ui;
    QS3Client *m_s3client;
    S3TreeModel *m_s3model;
    QFilesystemView *m_fsview;
    QList<QSharedPointer<ObjectHandlerInterface>> m_tasks;
    TransferTabWidget *m_transferTabWidget;
    QProgressDialog *quitDialog;
    QMenu *fileMenu;
    QAction *exitAction;

    QStringList downloadList;

    /*
     * File download/upload counters to indicate user that
     * whether all the files' transfer is successful.
     * TODO: Wrap these two flag into one struct, make the code more concise
    */
    int fileExists = 0;
    int fileFailed = 0;
};

#endif // MAINWINDOW_H
