/********************************************************/
// FILE: BackwardIntegrator.h
// DESCRIPTION: Raytracer Backward Integrator
// AUTHOR: Jan Schmid (jaschmid@eml.cc)    
/********************************************************/
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc/3.0/ or send 
// a letter to Creative Commons, 444 Castro Street, 
// Suite 900, Mountain View, California, 94041, USA.
/********************************************************/


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef RAYTRACE_BACKWARD_INTEGRATOR_GUARD
#define RAYTRACE_BACKWARD_INTEGRATOR_GUARD

#include <RaytraceCommon.h>
#include "IntegratorBase.h"
#include "static_vector.h"


namespace Raytrace {
	
template<class _SampleData,class _RayData,class _SceneReader,int _NumPathsPerBlock> struct BackwardIntegrator : public IntegratorBase<_SampleData,_RayData,_SceneReader>
{
	typedef Eigen::Array<Real,3,1> ColorArray;

	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _SceneReader SceneReader;

	typedef typename RayData::PrimitiveType PrimitiveType;
	typedef typename RayData::RayType RayType;

	typedef typename RayData::PrimitiveRelativeIntersection		IntersectionRelative;
	typedef typename RayData::AbsoluteIntersectionLocation		IntersectionAbsolute;

	typedef typename RayData::PrimitiveUserData					PrimitiveIdentifier;

	static const size_t ChunkSize = 8*1024;

	static const size_t NumPathsPerBlock = _NumPathsPerBlock;
	
	struct DirectNode
	{
		u32						_shadow;
		ColorArray				_color;
	};

	struct Path
	{
		typename SampleData::SampleInput		_sample;

		// some info

		size_t									_threadId;
		size_t									_firstDirect;
		size_t									_numDirect;

		// Intersection

		IntersectionRelative					_intersectionRelative;
		PrimitiveIdentifier						_id;
		IntersectionAbsolute					_intersectionAbsolute;
		
		// Rendering

		ColorArray								_accumulatedRadiance;

		Vector3									_parentDir;
		ColorArray								_pdf;
		Real									_cumulativeRoulette;
		Real									_rouletteThreshold;
	};
	
	
	static const size_t MaxEstimators = 4;
	
	struct Estimator
	{
		Vector3		_v;
		ColorArray	_factor;
		Real		_weight;
		Real		_pdf;
		Real		_pdfSum;
	};

	typedef chunked_vector<DirectNode,ChunkSize> DirectNodeArray;
	
	struct ThreadData
	{
		std::array<DirectNodeArray,2>	_directNodes;
	};

	typedef SplitUseWorkQueue<Path,NumPathsPerBlock> PathsArrayType;


	
	inline BackwardIntegrator() : _sampleData(nullptr),_rayData(nullptr),_writePathArray(0),_readPathArray(1)
	{
	}
	
	void InitializePrepareST(size_t numThreads,const SceneReader& scene,SampleData& sampleData,RayData& rayData) 
	{
		InitializeSceneData(scene);
		
		_threads.resize(numThreads);
		_pathArrays[_readPathArray].prepare(numThreads);
		_pathArrays[_writePathArray].prepare(numThreads);

		_camera.Initialize( Vector2(.75f,.75f));
		_sampleData = &sampleData;
		_rayData = &rayData;
	}
	void InitializeMT(size_t threadId) 
	{
	}
	void InitializeCompleteST() 
	{
	}

	void IntegratePrepareST() 
	{
	}

	void IntegrateMT(size_t threadId) 
	{

		ThreadData& threadDataWrite = _threads[threadId];
		DirectNodeArray& directNodeWrite = threadDataWrite._directNodes[_writePathArray];

		
		typename SampleData::SampleInput newSample;

		directNodeWrite.clear();

		// process new samples

		while(_sampleData->popGeneratedSample(threadId,newSample))
		{
			PathsArrayType& pathWrite = _pathArrays[_writePathArray];

			// create a new indirect node
			RayType cameraRay = _camera( _sampleData->getSampleValueImageXY(newSample,threadId));


			// create a new sample
			
			pathWrite.pushElement(Path(),threadId);
			Path* path = pathWrite.lastWriteElement(threadId);

			_rayData->pushRay(
				threadId,
				cameraRay,
				&path->_intersectionAbsolute,
				nullptr,
				&path->_intersectionRelative,
				&path->_id
				);
			
			path->_accumulatedRadiance = ColorArray(0.0f,0.0f,0.0f);
			path->_sample = newSample;
			path->_threadId = threadId;
			path->_firstDirect = -1;
			path->_numDirect = 0;
			path->_parentDir = -cameraRay.direction();
			path->_pdf = ColorArray(1.0f,1.0f,1.0f);
			path->_cumulativeRoulette = 1.0f;
			path->_rouletteThreshold = _sampleData->getSampleValueMisc(path->_sample,threadId);
		}


		PathsArrayType& pathRead = _pathArrays[_readPathArray];

		// process active samples

		// go to first element
		pathRead.advanceElement(threadId);

		Path* currPath;

		while(currPath = pathRead.currElement(threadId))
		{
			processPath(*currPath,threadId);

			pathRead.advanceElement(threadId);
		}
	}
	bool IntegrateCompleteST() 
	{
		_pathArrays[_readPathArray].clear();
		_pathArrays[_writePathArray].reset();

		size_t temp = _writePathArray;
		_writePathArray = _readPathArray;
		_readPathArray = temp;

		return true;
	}

	inline void processPath(Path& oldPath,size_t threadId)
	{
		static const Real SphereArea = (4.0f*M_PI);
		static const Real Epsilon = 0.00001f;

		PathsArrayType& pathWrite = _pathArrays[_writePathArray];
		ThreadData& threadDataWrite = _threads[threadId];
		ThreadData& threadDataRead = _threads[oldPath._threadId];
		const DirectNodeArray& directNodeRead = threadDataRead._directNodes[_readPathArray];
		
		GatherPathDirectLight(oldPath,directNodeRead);
		GatherPathEmissive(oldPath);

		// decide if we integrate further

		if( (oldPath._cumulativeRoulette <= oldPath._rouletteThreshold) || (oldPath._id == -1) || (oldPath._pdf[0] <= Epsilon && oldPath._pdf[1] <= Epsilon && oldPath._pdf[2] <= Epsilon) )
		{
			// was emitted
			completePath(oldPath,threadId);
			return;
		}
		else
		{
			Path* newPath = &oldPath;
			newPath->_numDirect = 0;
			DirectNodeArray& directNodeWrite = threadDataWrite._directNodes[_writePathArray];
			
				/*
			assert(newPath->_pdf.x() >= 0.0f);
			assert(newPath->_pdf.y() >= 0.0f);
			assert(newPath->_pdf.z() >= 0.0f);
			*/
			newPath->_threadId = threadId;

			GeneratePathDirectLight(*newPath,directNodeWrite);
				/*
			assert(newPath->_pdf.x() >= 0.0f);
			assert(newPath->_pdf.y() >= 0.0f);
			assert(newPath->_pdf.z() >= 0.0f);
			*/
			//newPath->_pdf /= SphereArea;//normalize integral
			
			const PrimitiveData& primitive = _primitives[newPath->_id];
			const MaterialSettings& material = _materials[primitive._material];

			ColorArray weightedAlbedo = Albedo(material,primitive,newPath->_parentDir)*newPath->_pdf;
			
			weightedAlbedo.x() = std::min(weightedAlbedo.x(),1.0f);
			weightedAlbedo.y() = std::min(weightedAlbedo.y(),1.0f);
			weightedAlbedo.z() = std::min(weightedAlbedo.z(),1.0f);

			Real reflectionChance = std::min<Real>(1.0f,std::max<Real>(0.0f,std::max<Real>(std::max<Real>(weightedAlbedo.x(),weightedAlbedo.y()),weightedAlbedo.z()))); 

			newPath->_cumulativeRoulette *= reflectionChance;

			//assert(newPath->_rouletteThreshold >= 0.0f);

			if(newPath->_cumulativeRoulette > newPath->_rouletteThreshold)
			{
				Vector3 reflectedDir;
				/*
				assert(newPath->_pdf.x() >= 0.0f);
				assert(newPath->_pdf.y() >= 0.0f);
				assert(newPath->_pdf.z() >= 0.0f);*/

				ColorArray  factor= GetReflectedRay(*newPath,threadId,reflectedDir);
			
				//newPath->_accumulatedRadiance = newPath->_pdf;
				/*completePath(*newPath,threadId);
				return;*/
				/*
				assert(pdf.x() >= 0.0f);
				assert(pdf.y() >= 0.0f);
				assert(pdf.z() >= 0.0f);*/

				newPath->_parentDir = -reflectedDir;

				//assert(reflectionChance > 0.0f);

				Vector3 oldPDF = newPath->_pdf;

				newPath->_pdf *= factor /reflectionChance;
				
				if(!( newPath->_pdf.x() >= 0.0f && newPath->_pdf.y() >= 0.0f && newPath->_pdf.z() >= 0.0f))
				{
					char temp[512];
					sprintf(temp,"oldPDF: %f/%f/%f \n newPDF: %f/%f/%f\n pdfMod: %f/%f/%f chance:%f\n",oldPDF.x(),oldPDF.y(),oldPDF.z(),newPath->_pdf.x(),newPath->_pdf.y(),newPath->_pdf.z(),factor.x(),factor.y(),factor.z(),reflectionChance);
					MessageBoxA(NULL,temp,"FAIL",MB_OK);
				}
				
				if(newPath->_pdf[0] > Epsilon || newPath->_pdf[1] > Epsilon || newPath->_pdf[2] > Epsilon)
				{
					
					pathWrite.pushElement(*newPath,threadId);
					newPath = pathWrite.lastWriteElement(threadId);

					RayType reflectedRay;

					reflectedRay.setOrigin(newPath->_intersectionAbsolute);
					reflectedRay.setDirection(reflectedDir);
					reflectedRay.setLength(-1.0f);

					_rayData->pushRay(
						threadId,
						reflectedRay,
						&newPath->_intersectionAbsolute,
						nullptr,
						&newPath->_intersectionRelative,
						&newPath->_id
						);
					return;
				}
			}

			if(newPath->_numDirect != 0)
				pathWrite.pushElement(*newPath,threadId);
			else
				completePath(*newPath,threadId);
		}
	}

	inline void completePath(const Path& path,size_t threadId)
	{
		_sampleData->pushCompletedSample( threadId, typename SampleData::SampleOutput( path._sample, path._accumulatedRadiance) );
	}


	inline ColorArray GetReflectedRay(Path& path,size_t threadId,Vector3& reflected)
	{

		// get estimators
		
		static_vector<Estimator,MaxEstimators> estimators;

		generateEstimators(path,estimators);

		// chose estimator
				
		Real random = _sampleData->getSampleValueMisc(path._sample,threadId);

		ColorArray chosenFactor;
		bool found = false;

		for(auto it = estimators.begin(); it != estimators.end(); ++it)
		{
			
			if(random < it->_weight)
			{
				if(it->_pdf)
					chosenFactor = it->_factor/it->_pdf;
				else
					chosenFactor = ColorArray(0.0f,0.0f,0.0f);
				//path._accumulatedRadiance = it->_debug;
				reflected = it->_v;
				found = true;
				break;
			}
			else
				random -= it->_weight;
		}
		if(!found)
			chosenFactor = ColorArray(0.0f,0.0f,0.0f);

		
		
		return chosenFactor;
	}
				
	
	inline void GatherPathEmissive(Path& path)
	{
		if(path._id != -1)
		{
			const PrimitiveData& primitive = _primitives[path._id];
			const MaterialSettings& material = _materials[primitive._material];
			
			// emitted light by the object itself
			path._accumulatedRadiance += material._color * material._emit * path._pdf;

		}
		else
		{
			// left scene, emissive = environment
			path._accumulatedRadiance += path._pdf * getBackgroundColor(-path._parentDir);
		}

	}

	inline void GeneratePathDirectLight(Path& path,DirectNodeArray& directNodeWrite)
	{
		const PrimitiveData& primitive = _primitives[path._id];
		const MaterialSettings& material = _materials[primitive._material];

		// direct lighting (added next step)
		path._firstDirect = directNodeWrite.size();
		for(auto il = _lights.begin(); il != _lights.end(); ++il)
		{
			Vector3 toLight = (il->_location - path._intersectionAbsolute);

			Real lightDistanceSq = toLight.squaredNorm();

			Real lightArea = lightDistanceSq*4*M_PI;

			toLight /= sqrtf(lightDistanceSq);


			ColorArray lightPDF = GetReflectedFactor(material,primitive,toLight,path._parentDir) * il->_color.array() * path._pdf / lightArea;
					
			if(lightPDF[0] >= Epsilon || lightPDF[1] >= Epsilon || lightPDF[2] >= Epsilon)
			{
				directNodeWrite.push_back(DirectNode());
				DirectNode& direct = directNodeWrite.back();
				direct._color = lightPDF;

				RayType ray;
							
				ray.setOrigin( path._intersectionAbsolute);
				ray.setDirection( (il->_location-path._intersectionAbsolute).normalized() );
				ray.setLength( (il->_location-path._intersectionAbsolute).norm() );

				_rayData->pushRay(path._threadId, ray, &direct._shadow);
			}
		}

		path._numDirect = directNodeWrite.size() - path._firstDirect;

	}

	inline void GatherPathDirectLight(Path& path,const DirectNodeArray& directNodeRead)
	{
		if(path._numDirect)
		{
			size_t end = path._firstDirect + path._numDirect;

			for(size_t i = path._firstDirect; i < end; ++i)
				if(!directNodeRead[i]._shadow)
					path._accumulatedRadiance +=  directNodeRead[i]._color;

		}
	}
	

	inline void generateEstimators(Path& path,static_vector<Estimator,MaxEstimators>& out)
	{
		const PrimitiveData& primitive = _primitives[path._id];
		const MaterialSettings& material = _materials[primitive._material];
		const Vector3& toViewer = path._parentDir;
		const Vector3& normal = primitive._normal;
		Vector3 yTangent = path._parentDir.cross(normal).normalized();
	
		if(yTangent.squaredNorm() != 0.0f)
			yTangent.normalize();
		else
		{
			yTangent = path._parentDir.cross(Vector3(1.0f,0.0f,0.0f));
			if(yTangent.squaredNorm() != 0.0f)
				yTangent.normalize();
			else
				yTangent = path._parentDir.cross(Vector3(0.0f,1.0f,0.0f)).normalized();
		}
		
		// random values
		Vector2 uv = _sampleData->getSampleValueMisc2D(path._sample,path._threadId);

		// variables
		Vector3 xTangent = yTangent.cross(normal).normalized();
		Vector3 xTangentSpecular;
		Vector3 xTangentRefracted;
		Vector3 vSpec;
		Vector3 vRef;
		Estimator e;

		bool hasDiffuse = false,hasReflect = false,hasTransparent=false,hasFallback=false;
		
		if(material._diffuseReflect > 0.0f)
		{
			e._pdf = cosineWeightedHemisphere(uv,xTangent,yTangent,normal,e._v);
			//e._debug = ColorArray(1.0f,0.0f,0.0f);
			hasDiffuse = true;
			e._weight =material._diffuseReflect;
			out.push_back(e);
		}		
		if(material._specularReflect > 0.0f && GetSpecularDirection(toViewer,primitive._normal,vSpec))
		{		
			xTangentSpecular = yTangent.cross(vSpec).normalized();

			e._pdf = cosineWeightedLobe(uv,xTangentSpecular,yTangent,vSpec,material._specularPower,e._v);
			//e._debug = ColorArray(0.0f,1.0f,0.0f);
			hasReflect = true;
			e._weight =material._specularReflect;
			out.push_back(e);
		}		
		if(material._transparency > 0.0f &&GetRefractedDirection(toViewer,primitive._normal,material._indexOfRefraction,vRef))
		{
			xTangentRefracted = yTangent.cross(vRef).normalized();

			e._pdf = cosineWeightedLobe(uv,xTangentRefracted,yTangent,vRef,material._refractionPower,e._v);
			//e._debug = ColorArray(0.0f,0.0f,1.0f);
			hasTransparent = true;
			e._weight =material._transparency;
			out.push_back(e);
		}
		{
			//fallback
			e._pdf = uniformSphere(uv,xTangent,yTangent,normal,e._v);
			//e._debug = ColorArray(0.5f,0.5f,.5f);
			hasFallback = true;
			e._weight = 0.1f;
			out.push_back(e);
		} 
		
		//calculate weights

		Real weightSum= 0.0f;

		for(auto it = out.begin(); it != out.end(); ++it)
		{
			it->_factor = GetReflectedFactor(material,primitive,it->_v,toViewer);
			Real max_factor = std::max(std::max(it->_factor.x(),it->_factor.y()),it->_factor.z());
			it->_factor *=  it->_pdf;
			it->_weight=max_factor;//*it->_pdf;
			weightSum += it->_weight;
		}
		
		//normalize weights
		if(weightSum <= 0.0f)
		{
			//if something goes wrong use fallback sphere
			out.back()._weight = 1.0f;
		}
		else
			for(auto it = out.begin(); it != out.end(); ++it)
			{
				it->_weight /= weightSum;
				it->_pdf *= it->_weight;
			}

		// calculate pdf's
			
		size_t iOut = 0;
		if(hasDiffuse)
		{
			Real pdfSum = 0.0f;
			for(auto it = out.begin(); it != out.end(); ++it)
				if(it - out.begin() != iOut)
					it->_pdf+= pdfAtCosineWeightedHemisphere(xTangent,yTangent,normal,it->_v)*out[iOut]._weight;

			++iOut;
		}
		if(hasReflect)
		{
			Real pdfSum = 0.0f;
			for(auto it = out.begin(); it != out.end(); ++it)
				if(it - out.begin() != iOut)
					it->_pdf+= pdfAtCosineWeightedLobe(xTangentSpecular,yTangent,vSpec,material._specularPower,it->_v)*out[iOut]._weight;

			++iOut;
		}
		if(hasTransparent)
		{
			Real pdfSum = 0.0f;
			for(auto it = out.begin(); it != out.end(); ++it)
				if(it - out.begin() != iOut)
					it->_pdf+= pdfAtCosineWeightedLobe(xTangentRefracted,yTangent,vRef,material._refractionPower,it->_v)*out[iOut]._weight;

			++iOut;
		}
		if(hasFallback)
		{
			Real pdfSum = 0.0f;
			for(auto it = out.begin(); it != out.end(); ++it)
				if(it - out.begin() != iOut)
					it->_pdf+= pdfAtUniformSphere(xTangent,yTangent,normal,it->_v)*out[iOut]._weight;

			++iOut;
		}

	}

	SampleData*						_sampleData;
	RayData*						_rayData;
	FisheyeCamera<RayType>			_camera;
	std::vector<ThreadData>			_threads;
	std::array<PathsArrayType,2>	_pathArrays;
	size_t							_readPathArray;
	size_t							_writePathArray;

};

}
#endif