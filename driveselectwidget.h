#ifndef DRIVESELECTWIDGET_H
#define DRIVESELECTWIDGET_H


#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
QT_END_NAMESPACE

class DriveSelectWidget : public QLineEdit
{
    Q_OBJECT
public:
    explicit DriveSelectWidget(QWidget *parent = 0);
    ~DriveSelectWidget();

signals:
    void driveChanged(QString);

private:
    QActionGroup *m_patternGroup;
};

#endif // DRIVESELECTWIDGET_H
