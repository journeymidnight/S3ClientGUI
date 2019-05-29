#ifndef TRANSFERWIDGET_H
#define TRANSFERWIDGET_H

#include <qtaskmodel.h>
#include <QTabWidget>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QItemDelegate>

class TransferTabWidget: public QTabWidget
{
    Q_OBJECT
public:
    TransferTabWidget(QWidget *parent = 0);
public:
    void addTask(QSharedPointer<TransferTask> t);
    int stopAll();
    int runningJobs();
signals:
    void TaskFinished(QSharedPointer<TransferTask> t);

private slots:
    void transferContextMenuRequest(const QPoint &);
    void redownload();

private:
    QTaskModel *m_taskModel;
    QTreeView *m_failedFileView;

    friend class TransferViewDelegate;
};

class TransferViewDelegate: public QItemDelegate
{
    Q_OBJECT
public:
    TransferViewDelegate(TransferTabWidget *widget): QItemDelegate(widget) {}
    /**/
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // TRANSFERWIDGET_H
