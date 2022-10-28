#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication::setAttribute( Qt::AA_UseHighDpiPixmaps, true );
  //QApplication::setAttribute( Qt::AA_EnableHighDpiScaling, true );
  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QApplication a(argc, argv);

	a.setApplicationName("uVisionCpp11Wizard");
	a.setOrganizationName("CS-Lab s.c.");
	a.setOrganizationDomain("cslab.com");

	MainWindow w;
	w.show();

	return a.exec();
}
