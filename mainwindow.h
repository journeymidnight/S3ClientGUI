#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../S3Client/qs3client.h"
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
    void on_S3ContextMenuRequest(const QPoint&);

    void on_LocalContextMenuRequest(const QPoint&);

    void on_download();
    void on_open();
    void on_upload();
    void on_delete();


    void on_taskFinished(QSharedPointer<TransferTask> t);
    void on_cmdFinished();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;


private:
    Ui::MainWindow *ui;
    QS3Client * m_s3client;
    S3TreeModel *m_s3model;
    QFilesystemView *m_fsview;
    QList<QSharedPointer<ObjectHandlerInterface>> m_tasks;
    TransferTabWidget *m_transferTabWidget;
    QProgressDialog *quitDialog;

};

#endif // MAINWINDOW_H
