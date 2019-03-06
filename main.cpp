#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/images/Main.png"));

    QFile qssFile(":/qss/default.qss");
	if (qssFile.open(QFile::ReadOnly)) {
		a.setStyleSheet(qssFile.readAll());
		qssFile.close();
	}

    MainWindow w;
    w.show();
    return a.exec();
}
