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
Date                   :=05/15/11
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
ObjectsFileList        :="/Users/mmg/Workspace_CL/wxMupen64Plus/wxMupen64Plus.txt"
MakeDirCommand         :=mkdir -p
CmpOptions             :=-Wall -g $(shell /usr/local/bin/wx-config-2.9 --cxxflags --unicode=yes --debug=yes core,base,gl) -arch i386 $(shell /usr/local/bin/sdl-config --cflags) -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.5.sdk $(Preprocessors)
C_CmpOptions           :=-Wall -g $(shell /usr/local/bin/wx-config-2.9 --cxxflags --unicode=yes --debug=yes core,base,gl) -arch i386 $(shell /usr/local/bin/sdl-config --cflags) -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.5.sdk $(Preprocessors)
LinkOptions            :=  -mwindows $(shell /usr/local/bin/wx-config-2.9 --debug=yes --unicode=yes --libs core,base,gl,html) -arch i386 $(shell /usr/local/bin/sdl-config --libs) -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.5.sdk
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)/Developer/hg/mupen64plus/mupen64plus-core/src/api $(IncludeSwitch)/Developer/hg/mupen64plus/mupen64plus-core/src 
RcIncludePath          :=
Libs                   :=
LibPath                := $(LibraryPathSwitch). 


##
## User defined environment variables
##
CodeLiteDir:=/Users/mmg/My Applications/Applications Dev/CodeLite.app/Contents/SharedSupport/
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/parameterpanel$(ObjectSuffix) $(IntermediateDirectory)/sdlkeypicker$(ObjectSuffix) $(IntermediateDirectory)/gamespanel$(ObjectSuffix) $(IntermediateDirectory)/sdlhelper$(ObjectSuffix) $(IntermediateDirectory)/config$(ObjectSuffix) $(IntermediateDirectory)/wxvidext$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix) \
	$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix) $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix) 

##
## Main Build Targets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep $(Objects)
	@$(MakeDirCommand) $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

objects_file:
	@echo $(Objects) > $(ObjectsFileList)

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
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/main.cpp"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/main.cpp"

$(IntermediateDirectory)/parameterpanel$(ObjectSuffix): parameterpanel.cpp $(IntermediateDirectory)/parameterpanel$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/parameterpanel.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/parameterpanel$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/parameterpanel$(DependSuffix): parameterpanel.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/parameterpanel$(ObjectSuffix) -MF$(IntermediateDirectory)/parameterpanel$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/parameterpanel.cpp"

$(IntermediateDirectory)/parameterpanel$(PreprocessSuffix): parameterpanel.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/parameterpanel$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/parameterpanel.cpp"

$(IntermediateDirectory)/sdlkeypicker$(ObjectSuffix): sdlkeypicker.cpp $(IntermediateDirectory)/sdlkeypicker$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/sdlkeypicker.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/sdlkeypicker$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sdlkeypicker$(DependSuffix): sdlkeypicker.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/sdlkeypicker$(ObjectSuffix) -MF$(IntermediateDirectory)/sdlkeypicker$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/sdlkeypicker.cpp"

$(IntermediateDirectory)/sdlkeypicker$(PreprocessSuffix): sdlkeypicker.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/sdlkeypicker$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/sdlkeypicker.cpp"

$(IntermediateDirectory)/gamespanel$(ObjectSuffix): gamespanel.cpp $(IntermediateDirectory)/gamespanel$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/gamespanel.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/gamespanel$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/gamespanel$(DependSuffix): gamespanel.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/gamespanel$(ObjectSuffix) -MF$(IntermediateDirectory)/gamespanel$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/gamespanel.cpp"

$(IntermediateDirectory)/gamespanel$(PreprocessSuffix): gamespanel.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/gamespanel$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/gamespanel.cpp"

$(IntermediateDirectory)/sdlhelper$(ObjectSuffix): sdlhelper.cpp $(IntermediateDirectory)/sdlhelper$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/sdlhelper.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/sdlhelper$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sdlhelper$(DependSuffix): sdlhelper.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/sdlhelper$(ObjectSuffix) -MF$(IntermediateDirectory)/sdlhelper$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/sdlhelper.cpp"

$(IntermediateDirectory)/sdlhelper$(PreprocessSuffix): sdlhelper.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/sdlhelper$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/sdlhelper.cpp"

$(IntermediateDirectory)/config$(ObjectSuffix): config.cpp $(IntermediateDirectory)/config$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/config.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/config$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/config$(DependSuffix): config.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/config$(ObjectSuffix) -MF$(IntermediateDirectory)/config$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/config.cpp"

$(IntermediateDirectory)/config$(PreprocessSuffix): config.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/config$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/config.cpp"

$(IntermediateDirectory)/wxvidext$(ObjectSuffix): wxvidext.cpp $(IntermediateDirectory)/wxvidext$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/wxvidext.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/wxvidext$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/wxvidext$(DependSuffix): wxvidext.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/wxvidext$(ObjectSuffix) -MF$(IntermediateDirectory)/wxvidext$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/wxvidext.cpp"

$(IntermediateDirectory)/wxvidext$(PreprocessSuffix): wxvidext.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/wxvidext$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/wxvidext.cpp"

$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix): mupen64plusplus/MupenAPI.c $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPI.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(DependSuffix): mupen64plusplus/MupenAPI.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPI.c"

$(IntermediateDirectory)/mupen64plusplus_MupenAPI$(PreprocessSuffix): mupen64plusplus/MupenAPI.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_MupenAPI$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPI.c"

$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix): mupen64plusplus/osal_files_unix.c $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_files_unix.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(DependSuffix): mupen64plusplus/osal_files_unix.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_files_unix.c"

$(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(PreprocessSuffix): mupen64plusplus/osal_files_unix.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_osal_files_unix$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_files_unix.c"

$(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix): mupen64plusplus/plugin.c $(IntermediateDirectory)/mupen64plusplus_plugin$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/plugin.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_plugin$(DependSuffix): mupen64plusplus/plugin.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mupen64plusplus_plugin$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_plugin$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/plugin.c"

$(IntermediateDirectory)/mupen64plusplus_plugin$(PreprocessSuffix): mupen64plusplus/plugin.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_plugin$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/plugin.c"

$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix): mupen64plusplus/osal_dynamiclib_unix.c $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_dynamiclib_unix.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(DependSuffix): mupen64plusplus/osal_dynamiclib_unix.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_dynamiclib_unix.c"

$(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(PreprocessSuffix): mupen64plusplus/osal_dynamiclib_unix.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mupen64plusplus_osal_dynamiclib_unix$(PreprocessSuffix) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/osal_dynamiclib_unix.c"

$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix): mupen64plusplus/MupenAPIpp.cpp $(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(DependSuffix)
	$(CompilerName) $(SourceSwitch) "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPIpp.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(DependSuffix): mupen64plusplus/MupenAPIpp.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mupen64plusplus_MupenAPIpp$(DependSuffix) -MM "/Users/mmg/Workspace_CL/wxMupen64Plus/mupen64plusplus/MupenAPIpp.cpp"

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
	$(RM) $(IntermediateDirectory)/sdlkeypicker$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/sdlkeypicker$(DependSuffix)
	$(RM) $(IntermediateDirectory)/sdlkeypicker$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/gamespanel$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/gamespanel$(DependSuffix)
	$(RM) $(IntermediateDirectory)/gamespanel$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/sdlhelper$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/sdlhelper$(DependSuffix)
	$(RM) $(IntermediateDirectory)/sdlhelper$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/config$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/config$(DependSuffix)
	$(RM) $(IntermediateDirectory)/config$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/wxvidext$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/wxvidext$(DependSuffix)
	$(RM) $(IntermediateDirectory)/wxvidext$(PreprocessSuffix)
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


