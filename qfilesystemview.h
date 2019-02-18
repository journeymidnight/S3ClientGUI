#ifndef QFILESYSTEMVIEW_H
#define QFILESYSTEMVIEW_H
#include <QFileSystemModel>
#include <QTreeView>


class QFilesystemView : public QTreeView
{
    Q_OBJECT
public:
    QFilesystemView(QWidget * parent=0);
    ~QFilesystemView();
    QString currentPath() const;
    void refreshSignals();
    QFileInfo currentFileInfo() const;
private slots:
    void changeToDir(const QModelIndex & index);
public slots:
    void upToParent();
signals:
    void rootPathChanged(const QString&);
    void updateInfo(const QString&);
private:
    QFileSystemModel *m_fsmodel;
};

#endif // QFILESYSTEMVIEW_H
