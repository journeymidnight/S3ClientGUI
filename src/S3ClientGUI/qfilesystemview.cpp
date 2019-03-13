#include "qfilesystemview.h"
#include <QHeaderView>
#include <QDir>
#include <QKeyEvent>
#include <QDebug>

QFilesystemView::QFilesystemView(QWidget *parent):QTreeView(parent)
{
    m_fsmodel = new FileSystemModel(this);
	m_fsmodel->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDot);


    setItemsExpandable(false);
    setRootIsDecorated(false);
    setExpandsOnDoubleClick(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
	header()->setSectionsMovable(false);

    //ui->verticalLayout->addWidget(new QTreeView(this));

    setModel(m_fsmodel);
    setRootIndex(m_fsmodel->setRootPath(dir.currentPath()));

	sortByColumn(0, Qt::AscendingOrder);

    //enter should key
    connect(this,SIGNAL(doubleClicked(QModelIndex)), this, SLOT(changeToDir(QModelIndex)));
	connect(m_fsmodel, SIGNAL(rootPathChanged(const QString &)), this, SIGNAL(rootPathChanged(const QString &)));
}


QFilesystemView::~QFilesystemView() {
    delete m_fsmodel;
}

void QFilesystemView::changeToDrive(const QString &drive)
{
	if (dir.cd(drive))
		setRootIndex(m_fsmodel->setRootPath(dir.absolutePath()));
}

void QFilesystemView::changeToDir(const QModelIndex &index) {
	QModelIndex idx = m_fsmodel->index(index.row(), 0, index.parent());
    if(m_fsmodel->isDir(idx) == true) {
		QString path = m_fsmodel->data(idx).toString();
		qDebug() << "QFilesystemView doubleClicked " << path;
		/*
		 * QDir("D:/").isRoot() return true;
		 * QDir("D:").isRoot() return false;
		 */
		if (dir.isRoot())
			path.append(QChar('/'));

		if (dir.cd(path)) {
			setRootIndex(m_fsmodel->setRootPath(dir.absolutePath()));
			emit updateInfo("now at new Dir");
		}
    }
}


void QFilesystemView::upToParent() {
	if (dir.isRoot())
		setRootIndex(m_fsmodel->setRootPath(m_fsmodel->myComputer().toString()));
	else if (dir.cdUp())
		setRootIndex(m_fsmodel->setRootPath(dir.absolutePath()));

	emit updateInfo("now at new Dir");
}

QString QFilesystemView::currentPath() const{
    QModelIndex index = rootIndex();
    return m_fsmodel->filePath(index);
}

void QFilesystemView::refreshSignals() {
    emit rootPathChanged(currentPath());
    emit updateInfo("now at new Dir");
}

QFileInfo QFilesystemView::currentFileInfo() const{
    return m_fsmodel->fileInfo(currentIndex());
}

void QFilesystemView::keyPressEvent(QKeyEvent *event)
{
	QModelIndex currentIndex = this->currentIndex();
	if (currentIndex.isValid()) {
		if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
			return changeToDir(currentIndex);
		else if (event->key() == Qt::Key_Backspace)
			return upToParent();
	}

	return QTreeView::keyPressEvent(event);
}