##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=wxMupen64Plus
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "/Users/mmg/Workspace_CL"
ProjectPath            := "/Users/mmg/Workspace_CL/wxMupen64Plus"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=mmg
Date                   :=10/02/10
CodeLitePath           :="/Users/mmg/Library/Application Support/codelite"
LinkerName             :=g++
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=g++ -dynamiclib -fPIC
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
OutputFile             :=$(IntermediateDirectory)/$(ProjectName).app/Contents/MacOS/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)__WX__ 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
MakeDirCommand         :=mkdir -p
CmpOptions             :=-Wall -g $(shell /usr/local/bin/wx-config-2.9 --cxxflags --unicode=yes --debug=yes) -arch i386 $(shell /usr/local/bin/sdl-config --cflags) $(Preprocessors)
LinkOptions            :=  -mwindows $(shell /usr/local/bin/wx-config-2.9 --debug=yes --libs --unicode=yes) -arch i386 $(shell /usr/local/bin/sdl-config --libs)
IncludePath            :=  "$(IncludeSwitch)." "$(IncludeSwitch)/Developer/hg/mupen64plus/mupen64plus-core/src/api" "$(IncludeSwitch)/Developer/hg/mupen64plus/mupen64plus-core/src" 
RcIncludePath          :=
Libs                   :=
LibPath                := "$(LibraryPathSwitch)." 


##
## User defined environment variables
##
CodeLiteDir:=/Applications/CodeLite.app/Contents/SharedSupport/
DYLD_LIBRARY_PATH:=/Developer/hg/mupen64plus/mupen64plus-core/projects/unix
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/parameterpanel$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix) 

##
## Main Build Targets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep $(Objects)
	@$(MakeDirCommand) $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

makeDirStep:
	@test -d ./Debug || $(MakeDirCommand) ./Debug
PrePreBuild: $(IntermediateDirectory)/$(ProjectName).app/Contents/Info.plist $(IntermediateDirectory)/$(ProjectName).app/Contents/Resources/wxMupen64Plus.icns
## rule to copy the Info.plist file into the bundle
$(IntermediateDirectory)/$(ProjectName).app/Contents/Info.plist: Info.plist
	mkdir -p '$(IntermediateDirectory)/$(ProjectName).app/Contents' && cp -f Info.plist '$(IntermediateDirectory)/$(ProjectName).app/Contents/Info.plist'
## rule to copy the icon file into the bundle
$(IntermediateDirectory)/$(ProjectName).app/Contents/Resources/wxMupen64Plus.icns: wxMupen64Plus.icns
	mkdir -p '$(IntermediateDirectory)/$(ProjectName).app/Contents/Resources/' && cp -f wxMupen64Plus.icns '$(IntermediateDirectory)/$(ProjectName).app/Contents/Resources/wxMupen64Plus.icns'

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/main.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/main.cpp"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/main.cpp"

$(IntermediateDirectory)/parameterpanel$(ObjectSuffix): parameterpanel.cpp $(IntermediateDirectory)/parameterpanel$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/parameterpanel.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/parameterpanel$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/parameterpanel$(DependSuffix): parameterpanel.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/parameterpanel$(ObjectSuffix) -MF$(IntermediateDirectory)/parameterpanel$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/parameterpanel.cpp"

$(IntermediateDirectory)/parameterpanel$(PreprocessSuffix): parameterpanel.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/parameterpanel$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/parameterpanel.cpp"

$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix): mupen64plusplus/MupenAPI.c $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPI.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(DependSuffix): mupen64plusplus/MupenAPI.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPI.c"

$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(PreprocessSuffix): mupen64plusplus/MupenAPI.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPI.c"

$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix): mupen64plusplus/osal_files_unix.c $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_files_unix.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(DependSuffix): mupen64plusplus/osal_files_unix.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_files_unix.c"

$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(PreprocessSuffix): mupen64plusplus/osal_files_unix.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_files_unix.c"

$(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix): mupen64plusplus/plugin.c $(IntermediateDirectory)/mupen64plusplus_plugin$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/plugin.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_plugin$(DependSuffix): mupen64plusplus/plugin.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_plugin$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/plugin.c"

$(IntermediateDirectory)/mupen64plusplus_plugin$(PreprocessSuffix): mupen64plusplus/plugin.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_plugin$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/plugin.c"

$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix): mupen64plusplus/osal_dynamiclib_unix.c $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_dynamiclib_unix.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(DependSuffix): mupen64plusplus/osal_dynamiclib_unix.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_dynamiclib_unix.c"

$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(PreprocessSuffix): mupen64plusplus/osal_dynamiclib_unix.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_dynamiclib_unix.c"

$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix): mupen64plusplus/MupenAPIpp.cpp $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPIpp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(DependSuffix): mupen64plusplus/MupenAPIpp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPIpp.cpp"

$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(PreprocessSuffix): mupen64plusplus/MupenAPIpp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPIpp.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/parameterpanel$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/parameterpanel$(DependSuffix)
	$(RM) $(IntermediateDirectory)/parameterpanel$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(DependSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(DependSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_plugin$(DependSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_plugin$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(DependSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(PreprocessSuffix)
	$(RM) $(OutputFile)


