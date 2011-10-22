#ifndef RAYTRACEGUI_H
#define RAYTRACEGUI_H

#include <QtGui/QMainWindow>
#include "ui_raytracegui.h"

class RaytraceGUI : public QMainWindow
{
	Q_OBJECT

public:
	RaytraceGUI(QWidget *parent = 0, Qt::WFlags flags = 0);
	~RaytraceGUI();

private:
	Ui::RaytraceGUIClass ui;
};

#endif // RAYTRACEGUI_H
