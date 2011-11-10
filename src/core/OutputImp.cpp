#include "headers.h"
#include "OutputImp.h"
#include "CameraImp.h"

namespace Raytrace {
RObjectType IOutput::ObjectType = ObjectType::Output;

Output CreateOutput(const String& name)
{
	return Output(new OutputImp(name));
}

OutputImp::OutputImp(const String& name) : Base(name),
	_enabled(true),
	_adaptiveRendering(true),
	_multisampleCount(1)
{
	_engineSettings._format = R8G8B8A8;
	_engineSettings._iXResolution = 0;
	_engineSettings._iYResolution = 0;
	_engineSettings._nDataSize = 0;
	_engineSettings._pDataOut = nullptr;
}
OutputImp::~OutputImp()
{
}

Result OutputImp::GetLastFrameTime(int & time)
{
	if(_raytraceEngine.get())
	{
		return _raytraceEngine->GetLastFrameTime(time);
	}
	else
		return Result::Failed;
}

Result OutputImp::SetOutputSurface(void* pData, int nDataSize, int xResolution, int yResolution, Format format)
{
	_engineSettings._iXResolution = xResolution;
	_engineSettings._iYResolution = yResolution;
	_engineSettings._nDataSize = nDataSize;
	_engineSettings._pDataOut = pData;
	switch(format)
	{
	case FORMAT_R8G8B8A8:
		_engineSettings._format = R8G8B8A8;
		break;
	case FORMAT_A8R8G8B8:
		_engineSettings._format = A8R8G8B8;
		break;
	default:
		_engineSettings._format = R8G8B8A8;
		break;
	}

	return Refresh();
}
Result OutputImp::Refresh()
{
	//try to retrieve camera and scene
	Scene scene;
	Camera camera;
	if(IObjectContainer* icamera = Base::_parent)
	{
		try
		{
			IObjectContainer* iscene = dynamic_cast<CameraImp*>(icamera)->GetScene();
			if(iscene != nullptr)
			{
				camera.reset(dynamic_cast<ICamera*>(icamera));
				scene.reset(dynamic_cast<IScene*>(iscene));
			}
		}
		catch(const std::bad_cast&)
		{
			camera.reset();
			scene.reset();
		}
	}

	if(scene.get() == nullptr || camera.get() == nullptr)
		return Result::Failed;

	_engineSettings._scene = scene;
	_engineSettings._camera = camera;
	_engineSettings._output.reset(this);

	if(_engineSettings._pDataOut == nullptr)
		_raytraceEngine.reset();
	else
	{
		if( _engine == RenderingEngineDummy)
			_raytraceEngine = CreateDummyEngine(_engineSettings);
		else if(_engine == RenderingEngineSimple)
			_raytraceEngine = CreateSimpleEngine(_engineSettings);
		else if(_engine == RenderingEngineFast)
			_raytraceEngine = CreateFastEngine(_engineSettings);
		else
			_raytraceEngine = CreateDummyEngine(_engineSettings);

		_raytraceEngine->Begin();
	}
	return Result::Succeeded;
}

Result OutputImp::UpdateOutput()
{
	if(_raytraceEngine.get())
	{
		return _raytraceEngine->Gather();
	}
	else
		return Result::Failed;
}

}