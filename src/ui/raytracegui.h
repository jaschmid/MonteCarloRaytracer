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
	void		previewSetMultisamples(int samples);
	void		previewSetCamera(QString camera);
	void		previewSetRenderingEngine(QString engine);
	void		updatePreviewOutput();
	void		previewOutputRefresh();

signals:

	void		sceneChanged(QString);
	void		previewMultisamplesChanged(int);
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
