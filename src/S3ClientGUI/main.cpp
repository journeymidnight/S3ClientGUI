#include "mainwindow.h"
#include "SingleApplication/singleapplication.h"
#include "qs3config.h"
#include "editaccountdialog.h"
#include <QFile>

int main(int argc, char *argv[])
{
	SingleApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/images/Main.png"));

    QFile qssFile(":/qss/default.qss");
	if (qssFile.open(QFile::ReadOnly)) {
		a.setStyleSheet(qssFile.readAll());
		qssFile.close();
	}

	QS3Config::Instance()->loadConfigFile();
	if (!QS3Config::Instance()->s3ConfigIsValid()) {
		EditAccountDialog *eaDlg = new EditAccountDialog();
		if (eaDlg->exec() == QDialog::Accepted) {
		}
		else
		{
			return 0;
		}
	}

    MainWindow w;
    w.show();
    return a.exec();
}
