#include "headers.h"
#include "MayaSceneReader.h"
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MPlugArray.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include "MayaGlobals.h"

inline void getCustomAttribute(MString &strAttribute, MString attribute, MFnDependencyNode &node)
{
	MPlug paramPlug;
	MStatus status;
	paramPlug = node.findPlug( attribute, &status );
	if ( status == MS::kSuccess ) {
		paramPlug.getValue( strAttribute );
	}else{
		strAttribute="";
	}
	return;
}

MayaSceneReader::MayaSceneReader() : _resolution(640,480)
{

	//scan the scene
	MItDag it(MItDag::kDepthFirst,MFn::kMesh);

	while(!it.isDone())
	{
 	
		// attach the function set to the object
		MFnMesh fn(it.item());

		// only want non-history items
		if( !fn.isIntermediateObject() ) 
		{
			parseTriMesh(it.item());
 			// print mesh name
			cout<<"Mesh "<< fn.name().asChar() <<endl;
		}

		// get next mesh
		it.next();

	}

	cout << "Number of primitives: " << GetNumPrimitives() << endl;

	{
		// environment image

		MString environmentFilename;
		MFnDependencyNode globals(raytraceGlobalsNode::get());
		getCustomAttribute(environmentFilename, "lightEnvironment", globals);

		if( Raytrace::String(environmentFilename.asChar()) != "")
		{
			cout << "Environment Radiance Map: " << environmentFilename.asChar() << endl;

			MImage img;
			MStatus filestat = img.readFromFile( environmentFilename, MImage::kFloat );
			unsigned int width, height;
			img.getSize(width,height);
			unsigned int depth = img.depth() / sizeof(float);
			cout << "width: "<< width << " height: " << height << " depth: " << depth << endl;
			_envSize = Raytrace::Vector2u(width,height);
			_envData.resize( _envSize.x() * _envSize.y() );
			float* pData = img.floatPixels();
			for(size_t y = 0; y < _envSize.y(); ++y)
				for(size_t x = 0; x < _envSize.x(); ++x)
				{
					int id = 0;
					for(;id< std::min<unsigned int>(4,depth); ++id)
						_envData[y * _envSize.x() + x][id] = pData[((height - y - 1) * _envSize.x() + x)*depth + id];
					if(id == 0)
						_envData[y * _envSize.x() + x][id++] = 1.0f;
					for(;id< 4; ++id)
						_envData[y * _envSize.x() + x][id] = _envData[y * _envSize.x() + x][id-1];
				}
		}
		else
		{
			_envSize = Raytrace::Vector2u(256,256);
			
			_envData.resize( _envSize.x() * _envSize.y() );
			for(size_t y = 0; y < _envSize.y(); ++y)
				for(size_t x = 0; x < _envSize.x(); ++x)
				{
					float py = (float)y / (float)(_envSize.y()-1);
					float px = (float)x / (float)(_envSize.x()-1);
					float f = cosf(py*M_PI/2.0f);
					_envData[y * _envSize.x() + x] = Raytrace::Vector4(1.0f,1.0f,1.0f,1.0f);
				}
		}
	}
}

MayaSceneReader::~MayaSceneReader()
{
}

size_t				MayaSceneReader::GetNumPrimitives() const
{
	if(_primitives.empty())
		return 0;
	else
	{
		auto it = _primitives.rbegin();
		return it->first.upper();
	}
}

void				MayaSceneReader::GetPrimitive(size_t i,void* pPrimitiveOut) const
{
	if(pPrimitiveOut == nullptr)
		return;

	PrimitiveTriangle* pOut = (PrimitiveTriangle*)pPrimitiveOut;
	int mat;
	auto it = _primitives.find(i);

	int lower = it->first.lower();
	
	const MeshContainer& meshCont = it->second;

	int primivite = (i - lower) %meshCont._primitesPerInstance;

	int instance = (i - lower) / meshCont._primitesPerInstance;
	
	MFnMesh mesh(meshCont._triMesh);

	Raytrace::Vector4 a,b,c;
	MPoint pA,pB,pC;
	mesh.getPoint(it->second._triangleVertices[primivite*3+0],pA);
	mesh.getPoint(it->second._triangleVertices[primivite*3+1],pB);
	mesh.getPoint(it->second._triangleVertices[primivite*3+2],pC);
	
	MMatrix localTransform = meshCont._dagPaths[instance].inclusiveMatrix();
	pA *= localTransform;
	pB *= localTransform;
	pC *= localTransform;

	a.x() = pA.x;
	a.y() = pA.y;
	a.z() = pA.z;
	a.w() = 1.0f;

	b.x() = pB.x;
	b.y() = pB.y;
	b.z() = pB.z;
	b.w() = 1.0f;

	c.x() = pC.x;
	c.y() = pC.y;
	c.z() = pC.z;
	c.w() = 1.0f;

	a = _viewMatrix*a;
	b = _viewMatrix*b;
	c = _viewMatrix*c;

	c = - c;
	b = - b;
	a = - a;

	pOut->_p1 = c.head<3>();
	pOut->_p2 = b.head<3>();
	pOut->_p3 = a.head<3>();
			
	if(meshCont._materialIndices[instance] >=0)
		pOut->_material = meshCont._materialIndices[instance];
	else
	{
		pOut->_material =  
			meshCont._faceMaterialIndices[instance][primivite];
	}

	return;
}


int MayaSceneReader::parseMaterial(const MObject& shadingEngine)
{
	MaterialContainer c;
	c._materialObject= shadingEngine;

	
	// attach a function set to the shading engine
	MFnDependencyNode fn( shadingEngine );

	// get access to the surfaceShader attribute. This will be connected to
	// lambert , phong nodes etc.
	MPlug sshader = fn.findPlug("surfaceShader");

	// will hold the connections to the surfaceShader attribute
	MPlugArray materials;

	// get the material connected to the surface shader
	sshader.connectedTo(materials,true,false);

	
	c._materialData._color = Raytrace::Vector3(0.0f,0.0f,0.0f);
	c._materialData._mirror_color = Raytrace::Vector3(1.0f,1.0f,1.0f);
	c._materialData._diffuseReflect = 0.0f;
	c._materialData._specular_reflect = 0.0f;
	c._materialData._emit = 0.0f;
	c._materialData._ior = 1.0f;
	c._materialData._transparency = 0.0f;
	c._materialData._transmit_filter = 0.0f;
	c._materialData._translucency_power = 200.0f;
	c._materialData._specular_power = 200.0f;

	// if we found a material
	if(materials.length() > 0)
	{

		switch(materials[0].node().apiType())
		{
		case MFn::kPhong:
		case MFn::kPhongMaterial:
			{
				// attach the function set to the object
				MFnPhongShader fn(materials[0].node());
				c._materialData._color = Raytrace::Vector3( fn.color().r,fn.color().g,fn.color().b);
				c._materialData._mirror_color = Raytrace::Vector3(fn.specularColor().r,fn.specularColor().g,fn.specularColor().b);
				c._materialData._diffuseReflect = fn.diffuseCoeff();
				c._materialData._specular_reflect = fn.reflectivity();
				c._materialData._emit = (fn.incandescence().r + fn.incandescence().g + fn.incandescence().b) / 3.0f;
				c._materialData._ior = fn.refractiveIndex();
				c._materialData._transparency = fn.translucenceCoeff();
				c._materialData._transmit_filter = Raytrace::Vector3( fn.transparency().r, fn.transparency().g, fn.transparency().b ).sum()/3.0f;
				c._materialData._translucency_power = 200.0f;
				c._materialData._specular_power = fn.cosPower();
			}
			break;
		case MFn::kLambert:
		case MFn::kLambertMaterial:
			{
				// attach the function set to the object
				MFnLambertShader fn(materials[0].node());
				c._materialData._color = Raytrace::Vector3( fn.color().r,fn.color().g,fn.color().b);
				c._materialData._mirror_color = Raytrace::Vector3(1.0f,1.0f,1.0f);
				c._materialData._diffuseReflect = fn.diffuseCoeff();
				c._materialData._specular_reflect = 0.0f;
				c._materialData._emit = (fn.incandescence().r + fn.incandescence().g + fn.incandescence().b) / 3.0f;
				c._materialData._ior = fn.refractiveIndex();
				c._materialData._transparency = fn.translucenceCoeff();
				c._materialData._transmit_filter = Raytrace::Vector3( fn.transparency().r, fn.transparency().g, fn.transparency().b ).sum()/3.0f;
				c._materialData._translucency_power = 200.0f;
				c._materialData._specular_power = 200.0f;
			}
			break;
		default:
			{
				std::cout<< "Unknown material: " << materials[0].node().apiTypeStr() << std::endl;
			}
			break;
		}
	}	

	_materials.push_back(c);
	return _materials.size() -1;
}

size_t				MayaSceneReader::GetNumMaterials() const
{
	return _materials.size();
}

void				MayaSceneReader::GetMaterial(size_t i,MayaSceneReader::MaterialData* pMaterialOut) const
{
	*pMaterialOut = _materials[i]._materialData;
	/*
	pMaterialOut->_color = Raytrace::Vector3(1.0f,1.0f,1.0f);
	pMaterialOut->_mirror_color = Raytrace::Vector3(1.0f,1.0f,1.0f);
	pMaterialOut->_diffuseReflect = 0.2f;
	pMaterialOut->_specular_reflect = 0.8f;
	pMaterialOut->_emit = .0f;
	pMaterialOut->_ior = 1.0f;
	pMaterialOut->_transparency = 0.0f;
	pMaterialOut->_translucency_power = 200.0f;
	pMaterialOut->_specular_power = 200.0f;*/
}

size_t				MayaSceneReader::GetNumLights() const
{
	return 0;
}

void				MayaSceneReader::GetLight(size_t i,MayaSceneReader::LightData* pLightOut) const
{
}

inline void getCustomAttribute(int &intAttribute, MString attribute, MFnDependencyNode &node)
{
	MPlug paramPlug;
	MStatus status;
	paramPlug = node.findPlug( attribute, &status );
	if ( status == MS::kSuccess ) {
		paramPlug.getValue( intAttribute );
	}else{
		intAttribute=0;
	}
	return;
}


Raytrace::Vector2u	MayaSceneReader::GetResolution() const
{
	return _resolution;
}
Raytrace::Real		MayaSceneReader::GetFieldOfView() const
{
	MFnCamera cam(_camera.node());
	return cam.verticalFieldOfView();
}
Raytrace::Real		MayaSceneReader::GetAspect() const
{
	MFnCamera cam(_camera.node());
	return cam.aspectRatio();
}
Raytrace::u32		MayaSceneReader::GetMultisampleCount() const
{
	int numSamples;
	MFnDependencyNode globals(raytraceGlobalsNode::get());
	getCustomAttribute(numSamples, "multisampleCount", globals);
	return (Raytrace::u32)numSamples;
}
Raytrace::String	MayaSceneReader::GetPrimitiveType() const
{
	return Raytrace::SceneReaderProperty_PrimitiveType_Triangle;
}

void MayaSceneReader::SetCamera(const MDagPath& camera)
{
	MMatrix matrix = camera.exclusiveMatrixInverse();

	for(int i = 0; i < 16; ++i)
		_viewMatrix(i/4,i%4) = matrix[i/4][i%4];

	//convert matrix format
	_viewMatrix.transposeInPlace();
	_camera = camera;
}

void MayaSceneReader::SetResolution(const Raytrace::Vector2u& resolution)
{
	_resolution = resolution;
}

Raytrace::Vector2u	MayaSceneReader::GetBackgroundRadianceSize() const
{
	return _envSize;
}

MayaSceneReader::Format	MayaSceneReader::GetBackgroundRadianceFormat() const
{
	return BACKGROUND_FORMAT_RGBA_F32;
}

void	MayaSceneReader::GetBackgroundRadianceData(void* pData) const
{
	memcpy(pData,_envData.data(),sizeof(Raytrace::Vector4)*_envData.size());
}

Raytrace::Matrix4 MayaSceneReader::GetViewMatrix() const
{
	return _viewMatrix;
}

void MayaSceneReader::parseTriMesh(const MObject& triMesh)
{
	MeshContainer container;

	container._triMesh = triMesh;

	MFnMesh mesh(triMesh);

	mesh.getTriangles( container._triangleCounts, container._triangleVertices);

	container._primitesPerInstance = container._triangleVertices.length()/3;
	mesh.getAllPaths(container._dagPaths);
	container._numInstances = container._dagPaths.length();

	int begin,end;
	if(_primitives.empty())
		begin = 0;
	else
	{
		auto it = _primitives.rbegin();
		begin = it->first.upper();
	}
	int num = container._primitesPerInstance * container._numInstances;
	end = begin + num;

	container._materialIndices.resize( container._numInstances );
	container._faceMaterialIndices.resize( container._numInstances );

	//resolve the shaders
	for(int i = 0; i < container._numInstances; ++i)
	{
		MObjectArray shaders;
		MIntArray    faceIndices;

		// get the shaders used by the i'th mesh instance
		mesh.getConnectedShaders(i,shaders,faceIndices);


		if(shaders.length() == 0)
			container._materialIndices[i] = 0;
		else
		{

			std::vector<int> localMaterials(shaders.length());

			for(int j = 0; j < shaders.length(); ++j)
			{
				localMaterials[j] = 0;
				bool found = false;
				for(int k = 0; k < _materials.size(); ++k)
					if(_materials[k]._materialObject == shaders[j])
					{
						localMaterials[j] = k;
						found = true;
						break;
					}

				if(!found)
					localMaterials[j] = parseMaterial(shaders[j]);
			}

			if(localMaterials.size() == 1)
				container._materialIndices[i] = localMaterials[0];
			else
			{
				container._materialIndices[i] = -1;
				container._faceMaterialIndices[i].resize( faceIndices.length() );
				faceIndices.get( container._faceMaterialIndices[i].data() );
				for(size_t j = 0; j < container._faceMaterialIndices[i].size(); ++ j)
					container._faceMaterialIndices[i][j] = localMaterials[
						container._faceMaterialIndices[i][j] ];
			}
		}
	}

	if(num != 0)
		_primitives.insert( std::make_pair(boost::icl::interval<int>::right_open( begin, end),container) );
}