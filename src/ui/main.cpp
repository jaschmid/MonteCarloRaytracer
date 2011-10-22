#include "headers.h"
#include "raytracegui.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	RaytraceGUI w;
	w.show();
	return a.exec();
}
