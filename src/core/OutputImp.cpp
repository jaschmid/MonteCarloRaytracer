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
	_engineSettings._format = ::Math::R8G8B8A8;
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
		_engineSettings._format = ::Math::R8G8B8A8;
		break;
	case FORMAT_A8R8G8B8:
		_engineSettings._format = ::Math::A8R8G8B8;
		break;
	default:
		_engineSettings._format = ::Math::R8G8B8A8;
		break;
	}

	return Refresh();
}
Result OutputImp::Refresh()
{
	Scene scene;
	if(IObjectContainer* icamera = Base::_parent)
	{
		IObjectContainer* iscene;
		try
		{
			iscene = dynamic_cast<CameraImp*>(icamera)->GetScene();
		}
		catch(const std::bad_cast&)
		{
			iscene= nullptr;
		}
		if(iscene)
		{
			try
			{
				scene = Scene(dynamic_cast<IScene*>(iscene));
			}
			catch(const std::bad_cast&)
			{
				scene.reset();
			}
		}
	}

	if(scene.get() == nullptr)
		return Result::Failed;

	_engineSettings._scene = scene;

	if(_engineSettings._pDataOut == nullptr)
		_raytraceEngine.reset();
	else
	{
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