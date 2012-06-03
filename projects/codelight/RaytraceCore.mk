##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=RaytraceCore
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/projects/codelight"
ProjectPath            := "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/projects/codelight"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Jan Schmid
Date                   :=12/09/2011
CodeLitePath           :="/home/jaschmid/.codelite"
LinkerName             :=g++
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
CompilerName           :=g++
C_CompilerName         :=gcc
OutputFile             :=$(IntermediateDirectory)/$(ProjectName).so
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="/home/jaschmid/Dropbox/School/TNCG15/Raytracer/projects/codelight/RaytraceCore.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
CmpOptions             := -g -std=gnu++0x $(Preprocessors)
C_CmpOptions           := -g $(Preprocessors)
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../src/include $(IncludeSwitch)../../src/src $(IncludeSwitch)/home/jaschmid/Code/eigen-eigen-3.0.3 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
LibPath                := $(LibraryPathSwitch). 


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects=$(IntermediateDirectory)/Core_XmlParserImp$(ObjectSuffix) $(IntermediateDirectory)/Core_WhittedIntegrator$(ObjectSuffix) $(IntermediateDirectory)/Core_TriMeshImp$(ObjectSuffix) $(IntermediateDirectory)/Core_BVHIntersector$(ObjectSuffix) $(IntermediateDirectory)/Core_CameraImp$(ObjectSuffix) $(IntermediateDirectory)/Core_EngineBase$(ObjectSuffix) $(IntermediateDirectory)/Core_Engines$(ObjectSuffix) $(IntermediateDirectory)/Core_headers$(ObjectSuffix) $(IntermediateDirectory)/Core_ImageWriter$(ObjectSuffix) $(IntermediateDirectory)/Core_MaterialImp$(ObjectSuffix) \
	$(IntermediateDirectory)/Core_MCSampler$(ObjectSuffix) $(IntermediateDirectory)/Core_ObjectTypeDefinitions$(ObjectSuffix) $(IntermediateDirectory)/Core_OutputImp$(ObjectSuffix) $(IntermediateDirectory)/Core_ResultDefinitions$(ObjectSuffix) $(IntermediateDirectory)/Core_SceneImp$(ObjectSuffix) $(IntermediateDirectory)/Core_SimpleEngine$(ObjectSuffix) $(IntermediateDirectory)/Core_SimpleIntersector$(ObjectSuffix) $(IntermediateDirectory)/TinyXml_tinystr$(ObjectSuffix) $(IntermediateDirectory)/TinyXml_tinyxml$(ObjectSuffix) $(IntermediateDirectory)/TinyXml_tinyxmlerror$(ObjectSuffix) \
	$(IntermediateDirectory)/TinyXml_tinyxmlparser$(ObjectSuffix) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects) > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@$(MakeDirCommand) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/projects/codelight/.build-debug"
	@echo rebuilt > "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/projects/codelight/.build-debug/RaytraceCore"

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/Core_XmlParserImp$(ObjectSuffix): ../../src/Core/XmlParserImp.cpp $(IntermediateDirectory)/Core_XmlParserImp$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/XmlParserImp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_XmlParserImp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_XmlParserImp$(DependSuffix): ../../src/Core/XmlParserImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_XmlParserImp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_XmlParserImp$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/XmlParserImp.cpp"

$(IntermediateDirectory)/Core_XmlParserImp$(PreprocessSuffix): ../../src/Core/XmlParserImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_XmlParserImp$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/XmlParserImp.cpp"

$(IntermediateDirectory)/Core_WhittedIntegrator$(ObjectSuffix): ../../src/Core/WhittedIntegrator.cpp $(IntermediateDirectory)/Core_WhittedIntegrator$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/WhittedIntegrator.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_WhittedIntegrator$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_WhittedIntegrator$(DependSuffix): ../../src/Core/WhittedIntegrator.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_WhittedIntegrator$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_WhittedIntegrator$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/WhittedIntegrator.cpp"

$(IntermediateDirectory)/Core_WhittedIntegrator$(PreprocessSuffix): ../../src/Core/WhittedIntegrator.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_WhittedIntegrator$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/WhittedIntegrator.cpp"

$(IntermediateDirectory)/Core_TriMeshImp$(ObjectSuffix): ../../src/Core/TriMeshImp.cpp $(IntermediateDirectory)/Core_TriMeshImp$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/TriMeshImp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_TriMeshImp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_TriMeshImp$(DependSuffix): ../../src/Core/TriMeshImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_TriMeshImp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_TriMeshImp$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/TriMeshImp.cpp"

$(IntermediateDirectory)/Core_TriMeshImp$(PreprocessSuffix): ../../src/Core/TriMeshImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_TriMeshImp$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/TriMeshImp.cpp"

$(IntermediateDirectory)/Core_BVHIntersector$(ObjectSuffix): ../../src/Core/BVHIntersector.cpp $(IntermediateDirectory)/Core_BVHIntersector$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/BVHIntersector.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_BVHIntersector$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_BVHIntersector$(DependSuffix): ../../src/Core/BVHIntersector.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_BVHIntersector$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_BVHIntersector$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/BVHIntersector.cpp"

$(IntermediateDirectory)/Core_BVHIntersector$(PreprocessSuffix): ../../src/Core/BVHIntersector.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_BVHIntersector$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/BVHIntersector.cpp"

$(IntermediateDirectory)/Core_CameraImp$(ObjectSuffix): ../../src/Core/CameraImp.cpp $(IntermediateDirectory)/Core_CameraImp$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/CameraImp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_CameraImp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_CameraImp$(DependSuffix): ../../src/Core/CameraImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_CameraImp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_CameraImp$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/CameraImp.cpp"

$(IntermediateDirectory)/Core_CameraImp$(PreprocessSuffix): ../../src/Core/CameraImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_CameraImp$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/CameraImp.cpp"

$(IntermediateDirectory)/Core_EngineBase$(ObjectSuffix): ../../src/Core/EngineBase.cpp $(IntermediateDirectory)/Core_EngineBase$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/EngineBase.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_EngineBase$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_EngineBase$(DependSuffix): ../../src/Core/EngineBase.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_EngineBase$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_EngineBase$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/EngineBase.cpp"

$(IntermediateDirectory)/Core_EngineBase$(PreprocessSuffix): ../../src/Core/EngineBase.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_EngineBase$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/EngineBase.cpp"

$(IntermediateDirectory)/Core_Engines$(ObjectSuffix): ../../src/Core/Engines.cpp $(IntermediateDirectory)/Core_Engines$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/Engines.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_Engines$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Engines$(DependSuffix): ../../src/Core/Engines.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Engines$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Engines$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/Engines.cpp"

$(IntermediateDirectory)/Core_Engines$(PreprocessSuffix): ../../src/Core/Engines.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Engines$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/Engines.cpp"

$(IntermediateDirectory)/Core_headers$(ObjectSuffix): ../../src/Core/headers.cpp $(IntermediateDirectory)/Core_headers$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/headers.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_headers$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_headers$(DependSuffix): ../../src/Core/headers.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_headers$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_headers$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/headers.cpp"

$(IntermediateDirectory)/Core_headers$(PreprocessSuffix): ../../src/Core/headers.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_headers$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/headers.cpp"

$(IntermediateDirectory)/Core_ImageWriter$(ObjectSuffix): ../../src/Core/ImageWriter.cpp $(IntermediateDirectory)/Core_ImageWriter$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ImageWriter.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_ImageWriter$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_ImageWriter$(DependSuffix): ../../src/Core/ImageWriter.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_ImageWriter$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_ImageWriter$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ImageWriter.cpp"

$(IntermediateDirectory)/Core_ImageWriter$(PreprocessSuffix): ../../src/Core/ImageWriter.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_ImageWriter$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ImageWriter.cpp"

$(IntermediateDirectory)/Core_MaterialImp$(ObjectSuffix): ../../src/Core/MaterialImp.cpp $(IntermediateDirectory)/Core_MaterialImp$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/MaterialImp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_MaterialImp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_MaterialImp$(DependSuffix): ../../src/Core/MaterialImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_MaterialImp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_MaterialImp$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/MaterialImp.cpp"

$(IntermediateDirectory)/Core_MaterialImp$(PreprocessSuffix): ../../src/Core/MaterialImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_MaterialImp$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/MaterialImp.cpp"

$(IntermediateDirectory)/Core_MCSampler$(ObjectSuffix): ../../src/Core/MCSampler.cpp $(IntermediateDirectory)/Core_MCSampler$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/MCSampler.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_MCSampler$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_MCSampler$(DependSuffix): ../../src/Core/MCSampler.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_MCSampler$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_MCSampler$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/MCSampler.cpp"

$(IntermediateDirectory)/Core_MCSampler$(PreprocessSuffix): ../../src/Core/MCSampler.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_MCSampler$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/MCSampler.cpp"

$(IntermediateDirectory)/Core_ObjectTypeDefinitions$(ObjectSuffix): ../../src/Core/ObjectTypeDefinitions.cpp $(IntermediateDirectory)/Core_ObjectTypeDefinitions$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ObjectTypeDefinitions.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_ObjectTypeDefinitions$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_ObjectTypeDefinitions$(DependSuffix): ../../src/Core/ObjectTypeDefinitions.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_ObjectTypeDefinitions$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_ObjectTypeDefinitions$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ObjectTypeDefinitions.cpp"

$(IntermediateDirectory)/Core_ObjectTypeDefinitions$(PreprocessSuffix): ../../src/Core/ObjectTypeDefinitions.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_ObjectTypeDefinitions$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ObjectTypeDefinitions.cpp"

$(IntermediateDirectory)/Core_OutputImp$(ObjectSuffix): ../../src/Core/OutputImp.cpp $(IntermediateDirectory)/Core_OutputImp$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/OutputImp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_OutputImp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_OutputImp$(DependSuffix): ../../src/Core/OutputImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_OutputImp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_OutputImp$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/OutputImp.cpp"

$(IntermediateDirectory)/Core_OutputImp$(PreprocessSuffix): ../../src/Core/OutputImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_OutputImp$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/OutputImp.cpp"

$(IntermediateDirectory)/Core_ResultDefinitions$(ObjectSuffix): ../../src/Core/ResultDefinitions.cpp $(IntermediateDirectory)/Core_ResultDefinitions$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ResultDefinitions.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_ResultDefinitions$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_ResultDefinitions$(DependSuffix): ../../src/Core/ResultDefinitions.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_ResultDefinitions$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_ResultDefinitions$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ResultDefinitions.cpp"

$(IntermediateDirectory)/Core_ResultDefinitions$(PreprocessSuffix): ../../src/Core/ResultDefinitions.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_ResultDefinitions$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/ResultDefinitions.cpp"

$(IntermediateDirectory)/Core_SceneImp$(ObjectSuffix): ../../src/Core/SceneImp.cpp $(IntermediateDirectory)/Core_SceneImp$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SceneImp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_SceneImp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_SceneImp$(DependSuffix): ../../src/Core/SceneImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_SceneImp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_SceneImp$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SceneImp.cpp"

$(IntermediateDirectory)/Core_SceneImp$(PreprocessSuffix): ../../src/Core/SceneImp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_SceneImp$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SceneImp.cpp"

$(IntermediateDirectory)/Core_SimpleEngine$(ObjectSuffix): ../../src/Core/SimpleEngine.cpp $(IntermediateDirectory)/Core_SimpleEngine$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SimpleEngine.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_SimpleEngine$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_SimpleEngine$(DependSuffix): ../../src/Core/SimpleEngine.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_SimpleEngine$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_SimpleEngine$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SimpleEngine.cpp"

$(IntermediateDirectory)/Core_SimpleEngine$(PreprocessSuffix): ../../src/Core/SimpleEngine.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_SimpleEngine$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SimpleEngine.cpp"

$(IntermediateDirectory)/Core_SimpleIntersector$(ObjectSuffix): ../../src/Core/SimpleIntersector.cpp $(IntermediateDirectory)/Core_SimpleIntersector$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SimpleIntersector.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/Core_SimpleIntersector$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_SimpleIntersector$(DependSuffix): ../../src/Core/SimpleIntersector.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_SimpleIntersector$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_SimpleIntersector$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SimpleIntersector.cpp"

$(IntermediateDirectory)/Core_SimpleIntersector$(PreprocessSuffix): ../../src/Core/SimpleIntersector.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_SimpleIntersector$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/Core/SimpleIntersector.cpp"

$(IntermediateDirectory)/TinyXml_tinystr$(ObjectSuffix): ../../src/TinyXml/tinystr.cpp $(IntermediateDirectory)/TinyXml_tinystr$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinystr.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/TinyXml_tinystr$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/TinyXml_tinystr$(DependSuffix): ../../src/TinyXml/tinystr.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/TinyXml_tinystr$(ObjectSuffix) -MF$(IntermediateDirectory)/TinyXml_tinystr$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinystr.cpp"

$(IntermediateDirectory)/TinyXml_tinystr$(PreprocessSuffix): ../../src/TinyXml/tinystr.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/TinyXml_tinystr$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinystr.cpp"

$(IntermediateDirectory)/TinyXml_tinyxml$(ObjectSuffix): ../../src/TinyXml/tinyxml.cpp $(IntermediateDirectory)/TinyXml_tinyxml$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxml.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/TinyXml_tinyxml$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/TinyXml_tinyxml$(DependSuffix): ../../src/TinyXml/tinyxml.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/TinyXml_tinyxml$(ObjectSuffix) -MF$(IntermediateDirectory)/TinyXml_tinyxml$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxml.cpp"

$(IntermediateDirectory)/TinyXml_tinyxml$(PreprocessSuffix): ../../src/TinyXml/tinyxml.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/TinyXml_tinyxml$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxml.cpp"

$(IntermediateDirectory)/TinyXml_tinyxmlerror$(ObjectSuffix): ../../src/TinyXml/tinyxmlerror.cpp $(IntermediateDirectory)/TinyXml_tinyxmlerror$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxmlerror.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/TinyXml_tinyxmlerror$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/TinyXml_tinyxmlerror$(DependSuffix): ../../src/TinyXml/tinyxmlerror.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/TinyXml_tinyxmlerror$(ObjectSuffix) -MF$(IntermediateDirectory)/TinyXml_tinyxmlerror$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxmlerror.cpp"

$(IntermediateDirectory)/TinyXml_tinyxmlerror$(PreprocessSuffix): ../../src/TinyXml/tinyxmlerror.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/TinyXml_tinyxmlerror$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxmlerror.cpp"

$(IntermediateDirectory)/TinyXml_tinyxmlparser$(ObjectSuffix): ../../src/TinyXml/tinyxmlparser.cpp $(IntermediateDirectory)/TinyXml_tinyxmlparser$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxmlparser.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/TinyXml_tinyxmlparser$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/TinyXml_tinyxmlparser$(DependSuffix): ../../src/TinyXml/tinyxmlparser.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/TinyXml_tinyxmlparser$(ObjectSuffix) -MF$(IntermediateDirectory)/TinyXml_tinyxmlparser$(DependSuffix) -MM "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxmlparser.cpp"

$(IntermediateDirectory)/TinyXml_tinyxmlparser$(PreprocessSuffix): ../../src/TinyXml/tinyxmlparser.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/TinyXml_tinyxmlparser$(PreprocessSuffix) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/src/TinyXml/tinyxmlparser.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/Core_XmlParserImp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_XmlParserImp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_XmlParserImp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_WhittedIntegrator$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_WhittedIntegrator$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_WhittedIntegrator$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_TriMeshImp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_TriMeshImp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_TriMeshImp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_BVHIntersector$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_BVHIntersector$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_BVHIntersector$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_CameraImp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_CameraImp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_CameraImp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_EngineBase$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_EngineBase$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_EngineBase$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_Engines$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_Engines$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_Engines$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_headers$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_headers$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_headers$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_ImageWriter$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_ImageWriter$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_ImageWriter$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_MaterialImp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_MaterialImp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_MaterialImp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_MCSampler$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_MCSampler$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_MCSampler$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_ObjectTypeDefinitions$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_ObjectTypeDefinitions$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_ObjectTypeDefinitions$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_OutputImp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_OutputImp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_OutputImp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_ResultDefinitions$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_ResultDefinitions$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_ResultDefinitions$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_SceneImp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_SceneImp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_SceneImp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_SimpleEngine$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_SimpleEngine$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_SimpleEngine$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_SimpleIntersector$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_SimpleIntersector$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_SimpleIntersector$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinystr$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinystr$(DependSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinystr$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxml$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxml$(DependSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxml$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxmlerror$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxmlerror$(DependSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxmlerror$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxmlparser$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxmlparser$(DependSuffix)
	$(RM) $(IntermediateDirectory)/TinyXml_tinyxmlparser$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) "/home/jaschmid/Dropbox/School/TNCG15/Raytracer/projects/codelight/.build-debug/RaytraceCore"


