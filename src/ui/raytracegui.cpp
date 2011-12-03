#include "headers.h"
#include "raytracegui.h"
#include <boost/lexical_cast.hpp>

static const char SceneFileFilter[] = "YafaRay Scene Files (*.xml)";

RaytraceGUI::RaytraceGUI(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),_previewCompleted(false)
{
	_ui.setupUi(this);
	updateSceneUI();
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updatePreviewOutput()));
	timer->start(500);
	previewOutputRefresh();
}

RaytraceGUI::~RaytraceGUI()
{

}

void RaytraceGUI::openScene()
{
	QFileDialog dialog(this);
	dialog.setNameFilter(tr(SceneFileFilter));
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setViewMode(QFileDialog::Detail);
	QStringList fileNames;
	if (dialog.exec())
		fileNames = dialog.selectedFiles();
	else
		return;

	if(fileNames.count() > 0)
	{
		openScene(Raytrace::String(fileNames.at(0).toAscii()));
	}
}

void RaytraceGUI::openScene(const Raytrace::String& string)
{
	_scene.reset();
	_previewCamera.reset();
	_previewOutput.reset();
	_previewCompleted = false;
	_renderCamera.reset();

	Raytrace::XmlParser xmlParser = Raytrace::CreateXmlParser();
	Raytrace::Result r = xmlParser->ParseFile(string,_scene);
	if(!r)
	{
		if(r == Raytrace::Result::ParsingError)
		{
			const Raytrace::String& error = xmlParser->GetError();
			_ui.statusBar->showMessage(QString(error.c_str()));
		}
		else
		{
			const Raytrace::String& error = (Raytrace::String)r;
			_ui.statusBar->showMessage(QString(error.c_str()));
		}
		return;
	}
	else
	{
		updateSceneUI();
	}
	return;
}

void RaytraceGUI::previewOutputRefresh()
{
	int width,height;

	width = _ui.renderArea->size().width();
	height = _ui.renderArea->size().height();
	

#ifdef _DEBUG
	width/=4;
	height/=4;
#endif

	if(!_outputImage.get() || (_outputImage->width() != width || _outputImage->height() != height) )
	{
		QImage* new_image = new QImage(width,height,QImage::Format_RGB32);
		_outputImage.reset( new_image );
	}

	if(_previewOutput.get())
		_previewOutput->SetOutputSurface( _outputImage->bits() , _outputImage->byteCount(), _outputImage->width(), _outputImage->height(), Raytrace::IOutput::FORMAT_A8R8G8B8);

	_previewCompleted = false;

	updatePreviewOutput();
}

void RaytraceGUI::updatePreviewOutput()
{
	if(_outputImage.get() != nullptr &&_previewOutput.get() != nullptr && !_previewCompleted)
	{
		std::string text;
		_previewOutput->GetLastFrameInfo(text);
		_ui.statusBar->showMessage(QString( Raytrace::String( text ).c_str()));

		Raytrace::Result res = _previewOutput->UpdateOutput();
		/*if(res == Raytrace::Result::RenderingComplete)
		{
			_previewCompleted = true;
			int time = -1;

		}
		else if(res == Raytrace::Result::RenderingInProgress)
			_ui.statusBar->showMessage(QString( Raytrace::String( Raytrace::String("Rendering In Progress") ).c_str()));
		else if(res == Raytrace::Result::Failed)
			_ui.statusBar->showMessage(QString( Raytrace::String( Raytrace::String("Rendering Not Ready Yet") ).c_str()));*/
		
		_ui.renderArea->setPixmap(nullptr);
		_ui.renderArea->setPixmap(QPixmap::fromImage( *_outputImage ) );
		//_ui.renderArea->resize( _outputImage->width(), _outputImage->height() );

	}

}

void		RaytraceGUI::previewRefresh()
{
	if(_previewOutput.get() != nullptr)
	{
		_previewCompleted = false;
		_previewOutput->Refresh();
	}
}

void		RaytraceGUI::previewSetCamera(QString camera)
{
	if(_scene.get() != nullptr)
	{
		Raytrace::Camera cameraObject = _scene->GetObject(camera.toStdString());
		if(cameraObject.get() != nullptr)
		{
			changePreviewCamera(cameraObject);
		}
	}
}


void		RaytraceGUI::previewSetRenderingEngine(QString engine)
{
	if(_previewOutput.get() != nullptr)
	{
		_previewOutput->SetEngine(engine.toStdString());
	}
}
void		RaytraceGUI::previewSetIntersector(QString engine)
{
	if(_previewOutput.get() != nullptr)
	{
		_previewOutput->SetIntersector(engine.toStdString());
	}
}
void		RaytraceGUI::previewSetIntegrator(QString engine)
{
	if(_previewOutput.get() != nullptr)
	{
		_previewOutput->SetIntegrator(engine.toStdString());
	}
}
void		RaytraceGUI::previewSetSampler(QString engine)
{
	if(_previewOutput.get() != nullptr)
	{
		_previewOutput->SetSampler(engine.toStdString());
	}
}

void RaytraceGUI::updateSceneUI()
{
	if(_scene.get() != nullptr)
	{
		_ui.sceneNameText->setText( QString( _scene->GetName().c_str() ) );
		_ui.previewCamera->clear();
		_ui.previewCamera->setEnabled(true);
		Raytrace::Camera c = _scene->GetFirstObject(Raytrace::ICamera::ObjectType);
		while(c)
		{
			_ui.previewCamera->addItem( QString(c->GetName().c_str()));
			c = _scene->GetNextObject(c,Raytrace::ICamera::ObjectType);
		}

		const Raytrace::String selected(_ui.previewCamera->currentText().toStdString());

		changePreviewCamera(_scene->GetObject(selected));
	}
	else
	{
		_ui.previewCamera->clear();
		_ui.previewCamera->setEnabled(false);
		_ui.sceneNameText->setText( QString( "None" ) );
	}

	updatePreviewUI();
}

void RaytraceGUI::changePreviewCamera(Raytrace::Camera camera)
{
	if(camera.get() == nullptr)
	{
		if(_previewOutput.get() != nullptr)
			_previewOutput->Remove();

		_previewOutput.reset();
		_previewCamera = camera;
		return;
	}

	if(_previewOutput.get() == nullptr)
		_previewOutput = Raytrace::CreateOutput("preview");

	_previewOutput->Remove();
	camera->InsertObject(_previewOutput);
	_previewCamera = camera;
	previewOutputRefresh();
	return;
}

void RaytraceGUI::updatePreviewUI()
{
	if(_previewOutput.get() != nullptr)
	{
		_ui.previewRefreshButton->setEnabled(true);
		_ui.previewRenderingEngine->setEnabled(true);
		_ui.previewRenderingEngine->clear();
		_ui.previewIntersector->setEnabled(true);
		_ui.previewIntersector->clear();
		_ui.previewIntegrator->setEnabled(true);
		_ui.previewIntegrator->clear();
		_ui.previewSampler->setEnabled(true);
		_ui.previewSampler->clear();
		for(int i = 0; i < _previewOutput->GetNumEngines(); ++i)
			_ui.previewRenderingEngine->addItem( QString( _previewOutput->GetEngineName(i).c_str() ) );
		for(int i = 0; i < _previewOutput->GetNumIntersectors(); ++i)
			_ui.previewIntersector->addItem( QString( _previewOutput->GetIntersectorName(i).c_str() ) );
		for(int i = 0; i < _previewOutput->GetNumIntegrators(); ++i)
			_ui.previewIntegrator->addItem( QString( _previewOutput->GetIntegratorName(i).c_str() ) );
		for(int i = 0; i < _previewOutput->GetNumSamplers(); ++i)
			_ui.previewSampler->addItem( QString( _previewOutput->GetSamplerName(i).c_str() ) );
	}
	else
	{
		_ui.previewRefreshButton->setEnabled(false);
		_ui.previewRenderingEngine->setEnabled(false);
		_ui.previewRenderingEngine->clear();
		_ui.previewIntersector->setEnabled(false);
		_ui.previewIntersector->clear();
		_ui.previewIntegrator->setEnabled(false);
		_ui.previewIntegrator->clear();
		_ui.previewSampler->setEnabled(false);
		_ui.previewSampler->clear();
	}
}

void RaytraceGUI::changeRenderCamera(Raytrace::Camera camera)
{
	_renderCamera = camera;
}
