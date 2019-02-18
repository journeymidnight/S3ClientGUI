#include "transferwidget.h"
#include <QApplication>
#include "qtaskmodel.h"

/*
        "Queueing",
        "Running",
        "Suspended",
        "Failed",
        "SuccessCompleted"
*/

TransferTabWidget::TransferTabWidget(QWidget *parent):QTabWidget(parent) {
    m_taskModel = new QTaskModel(this);

    QStringList title_groups;
    title_groups << "queued files" << "failed files" << "successful files";

    QStringList groups_filter;

    //This filter is according to TaskStatus
    groups_filter << "\\b(Queueing|Running|Suspended)\\b" << "Failed" << "SuccessCompleted";

    for (int i = 0 ; i < title_groups.size() ; i++ ) {
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(m_taskModel);
        proxyModel->setFilterKeyColumn(0);
        proxyModel->setFilterRole(Qt::UserRole);
        proxyModel->setFilterRegExp(groups_filter[i]);

        QTreeView * view = new QTreeView (this);
        view->setModel(proxyModel);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->setItemsExpandable(false);
        view->setRootIsDecorated(false);
        view->setExpandsOnDoubleClick(false);
        view->setSortingEnabled(true);
        addTab(view, title_groups[i]);


        view->setItemDelegate(new TransferViewDelegate(this));

    }
    connect(m_taskModel, SIGNAL(TaskFinished(QSharedPointer<TransferTask>)), this, SIGNAL(TaskFinished(QSharedPointer<TransferTask>)));

}

void TransferTabWidget::addTask(QSharedPointer<TransferTask> t) {
    m_taskModel->addTask(t);
}

void TransferViewDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {


    if (index.column() != PROGRESS_COLUMN) {
        QItemDelegate::paint(painter, option, index);
        return;
    }

    // From Qt5 Example/network/torrent/
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.rect = option.rect;
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    // Set the progress and text values of the style option.


    int progress = index.data().toInt();

    progressBarOption.progress = progress < 0 ? 0 : progress;
    progressBarOption.text = QString::asprintf("%d%%", progressBarOption.progress);

    // Draw the progress bar onto the view.
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}


int TransferTabWidget::stopAll() {
    return m_taskModel->stopAll();
}

int TransferTabWidget::runningJobs() {
    return m_taskModel->runningJobs();
}
