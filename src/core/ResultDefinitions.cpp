#include "headers.h"

namespace Raytrace {

	Result::ResultClass const Result::Undefined(0x0,String("Undefined Result"));
	Result::ResultClass const Result::Succeeded(0x80000001,String("Succeeded"));
	Result::ResultClass const Result::RenderingInProgress(0x80000002,String("Rendering in Progress"));
	Result::ResultClass const Result::RenderingComplete(0x80000003,String("Rendering Complete"));

	Result::ResultClass const Result::Failed(0x00000001,String("Unspecified Error"));
	Result::ResultClass const Result::UnsupportedObjectType(0x00000002,String("Unsupported Object Type"));
	Result::ResultClass const Result::PropertyNotFound(0x00000003,String("Property not Found"));
	Result::ResultClass const Result::NoOutputSet(0x00000004,String("No output set"));
	Result::ResultClass const Result::FileNotFound(0x00000005,String("File not Found"));
	Result::ResultClass const Result::ParsingError(0x00000006,String("Parsing Error"));

}