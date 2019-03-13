#ifndef QFILESYSTEMVIEW_H
#define QFILESYSTEMVIEW_H

#include <QTreeView>
#include <QDir>
#include "filesystemmodel.h"

class QFilesystemView : public QTreeView
{
    Q_OBJECT
public:
    QFilesystemView(QWidget * parent=0);
    ~QFilesystemView();
    QString currentPath() const;
    void refreshSignals();
    QFileInfo currentFileInfo() const;
protected:
	void keyPressEvent(QKeyEvent *event) override;
private slots:
    void changeToDir(const QModelIndex &index);
public slots:
	void changeToDrive(const QString &drive);
    void upToParent();
signals:
    void rootPathChanged(const QString&);
    void updateInfo(const QString&);
private:
    FileSystemModel *m_fsmodel;
	QDir dir;
};

#endif // QFILESYSTEMVIEW_H
