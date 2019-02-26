#include "driveselectwidget.h"

#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QWidgetAction>
#include <QIcon>
#include <QFileIconProvider>
#include <QDir>
#include <QFileInfoList>

DriveSelectWidget::DriveSelectWidget(QWidget *parent)
    : QLineEdit (parent)
    , m_patternGroup(new QActionGroup(this))
{
    setReadOnly(true);
    connect(m_patternGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        emit this->driveChanged(action->text());
    });
    connect(this, &QLineEdit::textChanged, this, [=](QString text) {
        QList<QAction *> actionList = m_patternGroup->actions();
        for (auto action : actionList) {
            if (text.startsWith(action->text())) {
                action->setChecked(true);
                break;
            }
        }
    });

    m_patternGroup->setExclusive(true);
    QMenu *menu = new QMenu(this);
    QFileInfoList driveList = QDir::drives();
    for (auto drive : driveList) {
        QString path = drive.path();
        // chop last separator
        if (path.endsWith(QChar('/')))
            path.chop(1);
        QAction *action = menu->addAction(path);
        action->setCheckable(true);
        m_patternGroup->addAction(action);
    }

    QFileIconProvider iconProvider;

    QToolButton *driveButton = new QToolButton;
#ifndef QT_NO_CURSOR
    driveButton->setCursor(Qt::ArrowCursor);
#endif
    driveButton->setFocusPolicy(Qt::NoFocus);
    driveButton->setStyleSheet("* { border: none; }");
    driveButton->setIcon(iconProvider.icon(QFileIconProvider::Drive));
    driveButton->setMenu(menu);
    driveButton->setPopupMode(QToolButton::InstantPopup);

    QWidgetAction *driveAction = new QWidgetAction(this);
    driveAction->setDefaultWidget(driveButton);
    addAction(driveAction, QLineEdit::LeadingPosition);
}

DriveSelectWidget::~DriveSelectWidget()
{

}
