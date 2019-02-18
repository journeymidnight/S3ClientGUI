#include "qfilesystemview.h"
#include <QDebug>

QFilesystemView::QFilesystemView(QWidget *parent):QTreeView(parent)
{
    m_fsmodel = new QFileSystemModel(this);

    setItemsExpandable(false);
    setRootIsDecorated(false);
    setExpandsOnDoubleClick(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);

    //ui->verticalLayout->addWidget(new QTreeView(this));

    setModel(m_fsmodel);
    setRootIndex(m_fsmodel->setRootPath("/"));

    //enter should key
    connect(this,SIGNAL(doubleClicked(QModelIndex)), this, SLOT(changeToDir(QModelIndex)));
}


QFilesystemView::~QFilesystemView() {
    delete m_fsmodel;
}

void QFilesystemView::changeToDir(const QModelIndex &index) {
    if(m_fsmodel->isDir(index) == true) {
        //always find the DIR
        QModelIndex dirIndex = index.sibling(index.row(),0);
        qDebug() << "Jump to :" << m_fsmodel->data(dirIndex);
        setRootIndex(dirIndex);
        emit rootPathChanged(m_fsmodel->filePath(dirIndex));
        emit updateInfo("now at new Dir");
    }
}


void QFilesystemView::upToParent() {
    QModelIndex index = rootIndex();
    QModelIndex parent = m_fsmodel->parent(index);
    if (parent.isValid()) {
        changeToDir(parent);
    }
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
