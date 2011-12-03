#include "headers.h"
#include "OutputImp.h"
#include "CameraImp.h"
#include "SceneReader.h"

namespace Raytrace {
RObjectType IOutput::ObjectType = ObjectType::Output;

Output CreateOutput(const String& name)
{
	return Output(new OutputImp(name));
}

OutputImp::OutputImp(const String& name) : Base(name),
	_enabled(true)
{
	_engine = GetEngineName(0);
	_sampler = GetSamplerName(0);
	_intersector = GetIntersectorName(0);
	_integrator = GetIntegratorName(0);
	_nDataOut = 0;
	_pDataOut = nullptr;
}
OutputImp::~OutputImp()
{
}

Result OutputImp::GetLastFrameInfo(std::string& info)
{

	if(_raytraceEngine.get())
	{	
		info = std::string("");
		_raytraceEngine->GetStatus("info",info);
		/*
		for(int i = 0; i < _raytraceEngine->GetNumModes(); ++i)
		{
			int thisTime;
			_raytraceEngine->GetLastFrameTime(i,thisTime);
			info+=_raytraceEngine->GetModeName(i) + ": " + boost::lexical_cast<std::string>(thisTime) + " ms";
			if(i != _raytraceEngine->GetNumModes()-1)
				info+=", ";
		}*/
		return Result::Succeeded;
	}
	else
		return Result::Failed;
}

Result OutputImp::SetOutputSurface(void* pData, int nDataSize, int xResolution, int yResolution, Format format)
{
	_xResOut = xResolution;
	_yResOut = yResolution;
	_nDataOut = nDataSize;
	_pDataOut = pData;
	switch(format)
	{
	case FORMAT_R8G8B8A8:
		_outputFormat = R8G8B8A8;
		break;
	case FORMAT_A8R8G8B8:
		_outputFormat = A8R8G8B8;
		break;
	default:
		_outputFormat = R8G8B8A8;
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

	boost::shared_ptr<LoadedSceneReader<SimpleTriangle>> reader(new LoadedSceneReader<SimpleTriangle>(scene,camera,Output(this),Vector2u((u32)_xResOut,(u32)_yResOut)));

	if(_raytraceEngine.get())
	{
		_raytraceEngine.reset();
	}

	if(_pDataOut == nullptr)
		_raytraceEngine.reset();
	else
	{
		auto intersector = DefaultEngine::getIntersectors().find(_intersector)->second();
		auto integrator = DefaultEngine::getIntegrators().find(_integrator)->second();
		auto sampler = DefaultEngine::getSamplers().find(_sampler)->second();

		_raytraceEngine = DefaultEngine::getEngines().find(_engine)->second(intersector,sampler,integrator,reader);

		_raytraceEngine->Begin();
	}
	return Result::Succeeded;
}

Result OutputImp::UpdateOutput()
{
	if(_raytraceEngine.get())
	{
		return _raytraceEngine->GatherPreview(_outputFormat,_xResOut,_yResOut,_pDataOut);
	}
	else
		return Result::Failed;
}

}