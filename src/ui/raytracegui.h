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


public slots:
	void		openScene();
	void		previewRefresh();
	void		previewSetCamera(QString camera);
	void		updatePreviewOutput();
	void		previewOutputRefresh();

	void		previewSetRenderingEngine(QString engine);
	void		previewSetIntersector(QString engine);
	void		previewSetIntegrator(QString engine);
	void		previewSetSampler(QString engine);
signals:

	void		sceneChanged(QString);
	void		previewCameraChanged(QString);

private:

	void openScene(const Raytrace::String& sceneFile);

	void updateSceneUI();

	void changePreviewCamera(Raytrace::Camera camera);
	void updatePreviewUI();
	void changeRenderCamera(Raytrace::Camera camera);

	std::auto_ptr<QImage>	_outputImage;

	Ui::RaytraceGUIClass	_ui;

	Raytrace::Scene			_scene;

	Raytrace::Camera		_previewCamera;
	Raytrace::Output		_previewOutput;
	bool					_previewCompleted;

	Raytrace::Camera		_renderCamera;
};

#endif // RAYTRACEGUI_H
