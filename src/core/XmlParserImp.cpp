#include "headers.h"
#include "XmlParserImp.h"
#include <boost/lexical_cast.hpp>

namespace Raytrace {

//since we're using a little different style of attributes, this function converts them
bool XmlParserImp::readProperty(const TiXmlElement& e,String& outProp)
{
	if(e.Attribute("sval"))
	{
		//string attribute
		outProp = String(e.Attribute("sval"));
		return true;
	}
	else if(e.Attribute("fval"))
	{
		//float attribute
		outProp = String(e.Attribute("fval"));
		return true;
	}
	else if(e.Attribute("bval"))
	{
		//bool attribute
		outProp = String(e.Attribute("bval"));
		return true;
	}
	else if(e.Attribute("ival"))
	{
		//int attribute
		outProp = String(e.Attribute("ival"));
		return true;
	}
	else if(e.Attribute("r")&&e.Attribute("g")&&e.Attribute("b")&&e.Attribute("a"))
	{
		//color attribute
		outProp = String(e.Attribute("r")) + String(" ") + String(e.Attribute("g")) + String(" ") + String(e.Attribute("b")) + String(" ") + String(e.Attribute("a"));
		return true;
	}
	else if(e.Attribute("x")&&e.Attribute("y")&&e.Attribute("z"))
	{
		//vector attribute
		outProp = String(e.Attribute("x")) + String(" ") + String(e.Attribute("y")) + String(" ") + String(e.Attribute("z"));
		return true;
	}
	else if(e.Attribute("a")&&e.Attribute("b")&&e.Attribute("c"))
	{
		//triangle attribute
		outProp = String(e.Attribute("a")) + String(" ") + String(e.Attribute("b")) + String(" ") + String(e.Attribute("c"));
		return true;
	}
	return false;
}

bool XmlParserImp::readCameraNode(const TiXmlElement& e,Camera& camera)
{
	if(e.ValueStr() != String("camera"))
		return false;

	String name;
	if(e.Attribute("name"))
		name = String(e.Attribute("name"));
	else
	{
		_error += String("Unnamed Camera (") + boost::lexical_cast<String,int>(e.Row()) + String(")");
		return false;
	}

	camera = CreateCamera(name);

	const TiXmlElement* child = e.FirstChildElement();
	while(child)
	{
		String propertyValue;
		String propertyName = child->ValueStr();
		if(readProperty(*child,propertyValue))
		{
			camera->SetPropertyValue(propertyName,propertyValue);
		}
		child = child->NextSiblingElement();
	}

	return true;
}

bool XmlParserImp::readSceneNode(const TiXmlElement& e,Scene& scene)
{
	if(e.ValueStr() != String("scene"))
		return false;

	String name;
	if(e.Attribute("name"))
		name = String(e.Attribute("name"));

	if(e.Attribute("type")&&e.Attribute("type")!=String("triangle"))
	{
		_error += String("Unsupported Scene Type (") + boost::lexical_cast<String,int>(e.Row()) + String(")");
		return false;
	}

	scene = CreateScene(name);

	_currentScene = scene;
	
	const TiXmlElement* child = e.FirstChildElement();
	while(child)
	{
		String nodeName = child->ValueStr();

		if(nodeName == "mesh")
		{
			//mesh
			TriMesh mesh;
			if(readTriMeshNode(*child,mesh))
				scene->InsertObject(mesh);
		}
		else if(nodeName == "material")
		{
			//material
			Material material;
			if(readMaterialNode(*child,material))
				scene->InsertObject(material);
		}
		else if(nodeName == "camera")
		{
			//camera
			Camera camera;
			if(readCameraNode(*child,camera))
				scene->InsertObject(camera);
		}

		child = child->NextSiblingElement();
	}

	_currentScene.reset();

	return true;
}

bool XmlParserImp::readTriMeshNode(const TiXmlElement& e,TriMesh& trimesh)
{
	if(e.ValueStr() != String("mesh"))
		return false;

	String name;
	if(e.Attribute("name"))
		name = String(e.Attribute("name"));

	trimesh = CreateTriMesh(name);
	
	Raytrace::Material current_material;

	const TiXmlElement* child = e.FirstChildElement();
	while(child)
	{
		if(child->ValueStr() == "p")
		{
			// a vertex!
			double location[3];
			child->Attribute("x",&location[0]);
			child->Attribute("y",&location[1]);
			child->Attribute("z",&location[2]);
			trimesh->PushVertex(Vector3(location[0],location[1],location[2]));
		}
		else if(child->ValueStr() == "set_material")
		{
			// change current material !
			String material_name(child->Attribute("sval"));
			
			Material m = _currentScene->GetObject(material_name);
			if(m.get() != nullptr)
				current_material = m;
		}
		else if(child->ValueStr() == "f")
		{
			// a face!
			Math::Vector3<int> vertices;
			child->Attribute("a",&vertices.x);
			child->Attribute("b",&vertices.y);
			child->Attribute("c",&vertices.z);
			trimesh->PushTriangle(vertices[0],vertices[1],vertices[2],current_material);
		}
		child = child->NextSiblingElement();
	}

	return true;
}

bool XmlParserImp::readMaterialNode(const TiXmlElement& e,Material& material)
{
	if(e.ValueStr() != String("material"))
		return false;

	String name;
	if(e.Attribute("name"))
		name = String(e.Attribute("name"));
	else
	{
		_error += String("Unnamed Material (") + boost::lexical_cast<String,int>(e.Row()) + String(")");
		return false;
	}

	material = CreateMaterial(name);

	const TiXmlElement* child = e.FirstChildElement();
	while(child)
	{
		String propertyValue;
		String propertyName = child->ValueStr();
		if(readProperty(*child,propertyValue))
		{
			material->SetPropertyValue(propertyName,propertyValue);
		}
		child = child->NextSiblingElement();
	}

	return true;
}

XmlParser CreateXmlParser()
{
	return XmlParser(new XmlParserImp());
}

XmlParserImp::XmlParserImp()
{
}

XmlParserImp::~XmlParserImp()
{
}

Result XmlParserImp::ParseFile(const String& filename,Scene& output)
{
	TiXmlDocument document;

	if(!document.LoadFile(filename))
		return Result::FileNotFound;

	if(document.RootElement())
	{
		if(readSceneNode(*document.RootElement(),output))
			return Result::Succeeded;
		else
			return Result::ParsingError;
	}
	else
	{
		_error = String("File does not contain a Root Node");
		return Result::ParsingError;
	}
}

}