#include "transferwidget.h"
#include "qtaskmodel.h"

#include <QApplication>
#include <QHeaderView>
#include <QProgressBar>
#include <QPainter>

/*
        "Queueing",
        "Running",
        "Suspended",
        "Failed",
        "SuccessCompleted"
*/

TransferTabWidget::TransferTabWidget(QWidget *parent): QTabWidget(parent)
{
    m_taskModel = new QTaskModel(this);

    QStringList title_groups;
    title_groups << tr("Queued Files") << tr("Failed Files") << tr("Successful Files");

    QStringList groups_filter;

    //This filter is according to TaskStatus
    groups_filter << "\\b(Queueing|Running|Suspended)\\b" << "Failed" << "SuccessCompleted";

    for (int i = 0 ; i < title_groups.size() ; i++ ) {
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(m_taskModel);
        proxyModel->setFilterKeyColumn(0);
        proxyModel->setFilterRole(Qt::UserRole);
        proxyModel->setFilterRegExp(groups_filter[i]);

        QTreeView *view = new QTreeView (this);
        view->setModel(proxyModel);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->setItemsExpandable(false);
        view->setRootIsDecorated(false);
        view->setExpandsOnDoubleClick(false);
        view->setSortingEnabled(true);
        //view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        view->header()->setStretchLastSection(true);
        view->setUniformRowHeights(true);
        addTab(view, title_groups[i]);


        view->setItemDelegate(new TransferViewDelegate(this));

    }
    connect(m_taskModel, SIGNAL(TaskFinished(QSharedPointer<TransferTask>)), this,
            SIGNAL(TaskFinished(QSharedPointer<TransferTask>)));
    //follow task
    connect(this, &TransferTabWidget::TaskFinished, this, [ = ](QSharedPointer<TransferTask> t) {
        if (t->status == TaskStatus::Failed || t->status == TaskStatus::ObjectAlreadyExists)
            setCurrentIndex(1);
        else if (t->status == TaskStatus::SuccessCompleted)
            setCurrentIndex(2);
        qDebug() << "emit m_transferTabWidget Taskfinished signal";
    });
}

void TransferTabWidget::addTask(QSharedPointer<TransferTask> t)
{
    setCurrentIndex(0);
    m_taskModel->addTask(t);
}

void TransferViewDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{


    if (index.column() != PROGRESS_COLUMN) {
        QItemDelegate::paint(painter, option, index);
        return;
    }

    // From Qt5 Example/network/torrent/
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    //progressBarOption.rect = option.rect;
    progressBarOption.rect = QRect(option.rect.x() + 5, option.rect.y() + 5, option.rect.width() - 10,
                                   5);
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    // Set the progress and text values of the style option.


    int progress = index.data().toInt();

    progressBarOption.progress = progress < 0 ? 0 : progress;
    //progressBarOption.text = QString::asprintf("%d%%", progressBarOption.progress);
    painter->drawText(QRect(option.rect.right() - 35, option.rect.bottom() - 16, 35, 16),
                      QString::asprintf("%d%%", progressBarOption.progress));

    // Draw the progress bar onto the view.
    QProgressBar progressbar;
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter,
                                       &progressbar);
}

QSize TransferViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (index.column() == PROGRESS_COLUMN)
        return QSize(150, 30);

    return QItemDelegate::sizeHint(option, index);
}

int TransferTabWidget::stopAll()
{
    return m_taskModel->stopAll();
}

int TransferTabWidget::runningJobs()
{
    return m_taskModel->runningJobs();
}
