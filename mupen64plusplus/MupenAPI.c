#include "m64p_types.h"#include "m64p_common.h"#include "m64p_frontend.h"#include "m64p_config.h"#include "m64p_debugger.h"
#include "version.h"#include "plugin.h"#include "main/main.h"#include "mupen64plusplus/osal_preproc.h"#include "mupen64plusplus/osal_dynamiclib.h"#include <stdio.h>#include <assert.h>#ifndef NULL
#define NULL 0#endif/** static (local) variables **/static m64p_handle l_ConfigCore = NULL;static m64p_handle l_ConfigVideo = NULL;static m64p_handle l_ConfigPlugins = NULL;//static const char *l_CoreLibPath = NULL;static const char *l_ConfigDirPath = NULL;//static const char *l_ROMFilepath = NULL;       // filepath of ROM to load & run at startup#if defined(SHAREDIR)  static const char *l_DataDirPath = SHAREDIR;#else  static const char *l_DataDirPath = NULL;#endif/* global data definitions */int g_CoreCapabilities;
/* definitions of pointers to Core common functions */
ptr_CoreErrorMessage    CoreErrorMessage = NULL;

/* definitions of pointers to Core front-end functions */
ptr_CoreStartup         CoreStartup = NULL;
ptr_CoreShutdown        CoreShutdown = NULL;
ptr_CoreAttachPlugin    CoreAttachPlugin = NULL;
ptr_CoreDetachPlugin    CoreDetachPlugin = NULL;
ptr_CoreDoCommand       CoreDoCommand = NULL;
ptr_CoreOverrideVidExt  CoreOverrideVidExt = NULL;
ptr_CoreAddCheat        CoreAddCheat = NULL;
ptr_CoreCheatEnabled    CoreCheatEnabled = NULL;

/* definitions of pointers to Core config functions */
ptr_ConfigListSections     ConfigListSections = NULL;
ptr_ConfigOpenSection      ConfigOpenSection = NULL;
ptr_ConfigListParameters   ConfigListParameters = NULL;
ptr_ConfigSaveFile         ConfigSaveFile = NULL;
ptr_ConfigSetParameter     ConfigSetParameter = NULL;
ptr_ConfigGetParameter     ConfigGetParameter = NULL;
ptr_ConfigGetParameterType ConfigGetParameterType = NULL;
ptr_ConfigGetParameterHelp ConfigGetParameterHelp = NULL;
ptr_ConfigSetDefaultInt    ConfigSetDefaultInt = NULL;
ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat = NULL;
ptr_ConfigSetDefaultBool   ConfigSetDefaultBool = NULL;
ptr_ConfigSetDefaultString ConfigSetDefaultString = NULL;
ptr_ConfigGetParamInt      ConfigGetParamInt = NULL;
ptr_ConfigGetParamFloat    ConfigGetParamFloat = NULL;
ptr_ConfigGetParamBool     ConfigGetParamBool = NULL;
ptr_ConfigGetParamString   ConfigGetParamString = NULL;

ptr_ConfigGetSharedDataFilepath ConfigGetSharedDataFilepath = NULL;
ptr_ConfigGetUserConfigPath     ConfigGetUserConfigPath = NULL;
ptr_ConfigGetUserDataPath       ConfigGetUserDataPath = NULL;
ptr_ConfigGetUserCachePath      ConfigGetUserCachePath = NULL;

/* definitions of pointers to Core debugger functions */
ptr_DebugSetCallbacks      DebugSetCallbacks = NULL;
ptr_DebugSetCoreCompare    DebugSetCoreCompare = NULL;
ptr_DebugSetRunState       DebugSetRunState = NULL;
ptr_DebugGetState          DebugGetState = NULL;
ptr_DebugStep              DebugStep = NULL;
ptr_DebugDecodeOp          DebugDecodeOp = NULL;
ptr_DebugMemGetRecompInfo  DebugMemGetRecompInfo = NULL;
ptr_DebugMemGetMemInfo     DebugMemGetMemInfo = NULL;
ptr_DebugMemGetPointer     DebugMemGetPointer = NULL;

ptr_DebugMemRead64         DebugMemRead64 = NULL;
ptr_DebugMemRead32         DebugMemRead32 = NULL;
ptr_DebugMemRead16         DebugMemRead16 = NULL;
ptr_DebugMemRead8          DebugMemRead8 = NULL;

ptr_DebugMemWrite64        DebugMemWrite64 = NULL;
ptr_DebugMemWrite32        DebugMemWrite32 = NULL;
ptr_DebugMemWrite16        DebugMemWrite16 = NULL;
ptr_DebugMemWrite8         DebugMemWrite8 = NULL;

ptr_DebugGetCPUDataPtr     DebugGetCPUDataPtr = NULL;
ptr_DebugBreakpointLookup  DebugBreakpointLookup = NULL;
ptr_DebugBreakpointCommand DebugBreakpointCommand = NULL;

/* global variables */
m64p_dynlib_handle CoreHandle = NULL;#define g_Verbose 0
/* functions */void DebugCallback(void *Context, int level, const char *message){    if (level <= 1)        printf("%s Error: %s\n", (const char *) Context, message);    else if (level == 2)        printf("%s Warning: %s\n", (const char *) Context, message);    else if (level == 3 || (level == 5 && g_Verbose))        printf("%s: %s\n", (const char *) Context, message);    else if (level == 4)        printf("%s Status: %s\n", (const char *) Context, message);    /* ignore the verbose info for now */}
m64p_error AttachCoreLib(const char *CoreLibFilepath)
{
    /* check if Core DLL is already attached */
    if (CoreHandle != NULL)
        return M64ERR_INVALID_STATE;

    /* load the DLL */
    m64p_error rval = M64ERR_INTERNAL;
    /* first, try a library path+name that was given on the command-line */
    if (CoreLibFilepath != NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, CoreLibFilepath);
    }
    /* then try a library path that was given at compile time */
#if defined(COREDIR)
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, COREDIR OSAL_DEFAULT_DYNLIB_FILENAME);
    }
#endif
    /* then try just the filename of the shared library, to let dlopen() look through the system lib dirs */
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, OSAL_DEFAULT_DYNLIB_FILENAME);
    }
    /* as a last-ditch effort, try loading library in current directory */
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, OSAL_CURRENT_DIR OSAL_DEFAULT_DYNLIB_FILENAME);
    }
    /* if we haven't found a good core library by now, then we're screwed */
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        fprintf(stderr, "AttachCoreLib() Error: failed to find Mupen64Plus Core library\n");
        CoreHandle = NULL;
        return M64ERR_INPUT_NOT_FOUND;
    }

    /* attach and call the PluginGetVersion function, check the Core and API versions for compatibility with this front-end */
    ptr_PluginGetVersion CoreVersionFunc;
    CoreVersionFunc = (ptr_PluginGetVersion) osal_dynlib_getproc(CoreHandle, "PluginGetVersion");
    if (CoreVersionFunc == NULL)
    {
        fprintf(stderr, "AttachCoreLib() Error: Shared library '%s' invalid; no PluginGetVersion() function found.\n", CoreLibFilepath);
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INPUT_INVALID;
    }
    m64p_plugin_type PluginType = (m64p_plugin_type) 0;
    int Compatible = 0;
    int CoreVersion = 0, APIVersion = 0;
    const char *CoreName = NULL;
    (*CoreVersionFunc)(&PluginType, &CoreVersion, &APIVersion, &CoreName, &g_CoreCapabilities);
    if (PluginType != M64PLUGIN_CORE)
        fprintf(stderr, "AttachCoreLib() Error: Shared library '%s' invalid; wrong plugin type %i.\n", CoreLibFilepath, (int) PluginType);
    else if (CoreVersion < MINIMUM_CORE_VERSION)
        fprintf(stderr, "AttachCoreLib() Error: Shared library '%s' invalid; core version %i.%i.%i is below minimum supported %i.%i.%i\n",
                CoreLibFilepath, VERSION_PRINTF_SPLIT(CoreVersion), VERSION_PRINTF_SPLIT(MINIMUM_CORE_VERSION));
    else if (APIVersion < MINIMUM_API_VERSION)
        fprintf(stderr, "AttachCoreLib() Error: Shared library '%s' invalid; core API version %i.%i.%i is below minimum supported %i.%i.%i\n",
                CoreLibFilepath, VERSION_PRINTF_SPLIT(APIVersion), VERSION_PRINTF_SPLIT(MINIMUM_API_VERSION));
    else
        Compatible = 1;
    /* exit if not compatible */
    if (Compatible == 0)
    {
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INPUT_INVALID;
    }

    /* print some information about the core library */
    printf("wxMupen64Plus: attached to core library '%s' version %i.%i.%i\n", CoreName, VERSION_PRINTF_SPLIT(CoreVersion));
    if (g_CoreCapabilities & M64CAPS_DYNAREC)
        printf("            Includes support for Dynamic Recompiler.\n");
    if (g_CoreCapabilities & M64CAPS_DEBUGGER)
        printf("            Includes support for MIPS r4300 Debugger.\n");
    if (g_CoreCapabilities & M64CAPS_CORE_COMPARE)
        printf("            Includes support for r4300 Core Comparison.\n");

    /* get function pointers to the common and front-end functions */
    CoreErrorMessage = (ptr_CoreErrorMessage) osal_dynlib_getproc(CoreHandle, "CoreErrorMessage");
    CoreStartup = (ptr_CoreStartup) osal_dynlib_getproc(CoreHandle, "CoreStartup");
    CoreShutdown = (ptr_CoreShutdown) osal_dynlib_getproc(CoreHandle, "CoreShutdown");
    CoreAttachPlugin = (ptr_CoreAttachPlugin) osal_dynlib_getproc(CoreHandle, "CoreAttachPlugin");
    CoreDetachPlugin = (ptr_CoreDetachPlugin) osal_dynlib_getproc(CoreHandle, "CoreDetachPlugin");
    CoreDoCommand = (ptr_CoreDoCommand) osal_dynlib_getproc(CoreHandle, "CoreDoCommand");
    CoreOverrideVidExt = (ptr_CoreOverrideVidExt) osal_dynlib_getproc(CoreHandle, "CoreOverrideVidExt");
    CoreAddCheat = (ptr_CoreAddCheat) osal_dynlib_getproc(CoreHandle, "CoreAddCheat");
    CoreCheatEnabled = (ptr_CoreCheatEnabled) osal_dynlib_getproc(CoreHandle, "CoreCheatEnabled");

    /* get function pointers to the configuration functions */
    ConfigListSections = (ptr_ConfigListSections) osal_dynlib_getproc(CoreHandle, "ConfigListSections");
    ConfigOpenSection = (ptr_ConfigOpenSection) osal_dynlib_getproc(CoreHandle, "ConfigOpenSection");
    ConfigListParameters = (ptr_ConfigListParameters) osal_dynlib_getproc(CoreHandle, "ConfigListParameters");
    ConfigSaveFile = (ptr_ConfigSaveFile) osal_dynlib_getproc(CoreHandle, "ConfigSaveFile");
    ConfigSetParameter = (ptr_ConfigSetParameter) osal_dynlib_getproc(CoreHandle, "ConfigSetParameter");
    ConfigGetParameter = (ptr_ConfigGetParameter) osal_dynlib_getproc(CoreHandle, "ConfigGetParameter");
    ConfigGetParameterType = (ptr_ConfigGetParameterType) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterType");
    ConfigGetParameterHelp = (ptr_ConfigGetParameterHelp) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterHelp");
    ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultInt");
    ConfigSetDefaultFloat = (ptr_ConfigSetDefaultFloat) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultFloat");
    ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultBool");
    ConfigSetDefaultString = (ptr_ConfigSetDefaultString) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultString");
    ConfigGetParamInt = (ptr_ConfigGetParamInt) osal_dynlib_getproc(CoreHandle, "ConfigGetParamInt");
    ConfigGetParamFloat = (ptr_ConfigGetParamFloat) osal_dynlib_getproc(CoreHandle, "ConfigGetParamFloat");
    ConfigGetParamBool = (ptr_ConfigGetParamBool) osal_dynlib_getproc(CoreHandle, "ConfigGetParamBool");
    ConfigGetParamString = (ptr_ConfigGetParamString) osal_dynlib_getproc(CoreHandle, "ConfigGetParamString");

    ConfigGetSharedDataFilepath = (ptr_ConfigGetSharedDataFilepath) osal_dynlib_getproc(CoreHandle, "ConfigGetSharedDataFilepath");
    ConfigGetUserConfigPath = (ptr_ConfigGetUserConfigPath) osal_dynlib_getproc(CoreHandle, "ConfigGetUserConfigPath");
    ConfigGetUserDataPath = (ptr_ConfigGetUserDataPath) osal_dynlib_getproc(CoreHandle, "ConfigGetUserDataPath");
    ConfigGetUserCachePath = (ptr_ConfigGetUserCachePath) osal_dynlib_getproc(CoreHandle, "ConfigGetUserCachePath");

    /* get function pointers to the debugger functions */
    DebugSetCallbacks = (ptr_DebugSetCallbacks) osal_dynlib_getproc(CoreHandle, "DebugSetCallbacks");
    DebugSetCoreCompare = (ptr_DebugSetCoreCompare) osal_dynlib_getproc(CoreHandle, "DebugSetCoreCompare");
    DebugSetRunState = (ptr_DebugSetRunState) osal_dynlib_getproc(CoreHandle, "DebugSetRunState");
    DebugGetState = (ptr_DebugGetState) osal_dynlib_getproc(CoreHandle, "DebugGetState");
    DebugStep = (ptr_DebugStep) osal_dynlib_getproc(CoreHandle, "DebugStep");
    DebugDecodeOp = (ptr_DebugDecodeOp) osal_dynlib_getproc(CoreHandle, "DebugDecodeOp");
    DebugMemGetRecompInfo = (ptr_DebugMemGetRecompInfo) osal_dynlib_getproc(CoreHandle, "DebugMemGetRecompInfo");
    DebugMemGetMemInfo = (ptr_DebugMemGetMemInfo) osal_dynlib_getproc(CoreHandle, "DebugMemGetMemInfo");
    DebugMemGetPointer = (ptr_DebugMemGetPointer) osal_dynlib_getproc(CoreHandle, "DebugMemGetPointer");

    DebugMemRead64 = (ptr_DebugMemRead64) osal_dynlib_getproc(CoreHandle, "DebugMemRead64");
    DebugMemRead32 = (ptr_DebugMemRead32) osal_dynlib_getproc(CoreHandle, "DebugMemRead32");
    DebugMemRead16 = (ptr_DebugMemRead16) osal_dynlib_getproc(CoreHandle, "DebugMemRead16");
    DebugMemRead8 = (ptr_DebugMemRead8) osal_dynlib_getproc(CoreHandle, "DebugMemRead8");

    DebugMemWrite64 = (ptr_DebugMemWrite64) osal_dynlib_getproc(CoreHandle, "DebugMemRead64");
    DebugMemWrite32 = (ptr_DebugMemWrite32) osal_dynlib_getproc(CoreHandle, "DebugMemRead32");
    DebugMemWrite16 = (ptr_DebugMemWrite16) osal_dynlib_getproc(CoreHandle, "DebugMemRead16");
    DebugMemWrite8 = (ptr_DebugMemWrite8) osal_dynlib_getproc(CoreHandle, "DebugMemRead8");

    DebugGetCPUDataPtr = (ptr_DebugGetCPUDataPtr) osal_dynlib_getproc(CoreHandle, "DebugGetCPUDataPtr");
    DebugBreakpointLookup = (ptr_DebugBreakpointLookup) osal_dynlib_getproc(CoreHandle, "DebugBreakpointLookup");
    DebugBreakpointCommand = (ptr_DebugBreakpointCommand) osal_dynlib_getproc(CoreHandle, "DebugBreakpointCommand");

    return M64ERR_SUCCESS;
}

m64p_error DetachCoreLib(void)
{
    if (CoreHandle == NULL)
        return M64ERR_INVALID_STATE;

    /* set the core function pointers to NULL */
    CoreErrorMessage = NULL;
    CoreStartup = NULL;
    CoreShutdown = NULL;
    CoreAttachPlugin = NULL;
    CoreDetachPlugin = NULL;
    CoreDoCommand = NULL;
    CoreOverrideVidExt = NULL;
    CoreAddCheat = NULL;
    CoreCheatEnabled = NULL;

    ConfigListSections = NULL;
    ConfigOpenSection = NULL;
    ConfigListParameters = NULL;
    ConfigSetParameter = NULL;
    ConfigGetParameter = NULL;
    ConfigGetParameterType = NULL;
    ConfigGetParameterHelp = NULL;
    ConfigSetDefaultInt = NULL;
    ConfigSetDefaultBool = NULL;
    ConfigSetDefaultString = NULL;
    ConfigGetParamInt = NULL;
    ConfigGetParamBool = NULL;
    ConfigGetParamString = NULL;

    ConfigGetSharedDataFilepath = NULL;
    ConfigGetUserDataPath = NULL;
    ConfigGetUserCachePath = NULL;

    DebugSetCallbacks = NULL;
    DebugSetCoreCompare = NULL;
    DebugSetRunState = NULL;
    DebugGetState = NULL;
    DebugStep = NULL;
    DebugDecodeOp = NULL;
    DebugMemGetRecompInfo = NULL;
    DebugMemGetMemInfo = NULL;
    DebugMemGetPointer = NULL;

    DebugMemRead64 = NULL;
    DebugMemRead32 = NULL;
    DebugMemRead16 = NULL;
    DebugMemRead8 = NULL;

    DebugMemWrite64 = NULL;
    DebugMemWrite32 = NULL;
    DebugMemWrite16 = NULL;
    DebugMemWrite8 = NULL;

    DebugGetCPUDataPtr = NULL;
    DebugBreakpointLookup = NULL;
    DebugBreakpointCommand = NULL;

    /* detach the shared library */
    osal_dynlib_close(CoreHandle);
    CoreHandle = NULL;

    return M64ERR_SUCCESS;
}
m64p_handle getConfigUI(){    assert(l_ConfigPlugins != NULL);    return l_ConfigPlugins;}
m64p_error OpenConfigurationHandles(void)
{
    m64p_error rval;

    /* Open Configuration sections for core library and console User Interface */
    rval = (*ConfigOpenSection)("Core", &l_ConfigCore);
    if (rval != M64ERR_SUCCESS)
    {
        fprintf(stderr, "Error (%s:%i): failed to open 'Core' configuration section\n", __FILE__, __LINE__);
        return rval;
    }

    rval = (*ConfigOpenSection)("Video-General", &l_ConfigVideo);
    if (rval != M64ERR_SUCCESS)
    {
        fprintf(stderr, "Error (%s:%i): failed to open 'Video-General' configuration section\n", __FILE__, __LINE__);
        return rval;
    }
    rval = (*ConfigOpenSection)("UI-Plugins", &l_ConfigPlugins);
    if (rval != M64ERR_SUCCESS)
    {
        fprintf(stderr, "Error (%s:%i): failed to open 'UI-Plugins' configuration section\n", __FILE__, __LINE__);
        return rval;
    }

    /* Set default values for my Config parameters */
    (*ConfigSetDefaultString)(l_ConfigPlugins, "PluginDir", OSAL_CURRENT_DIR, "Directory in which to search for plugins");
    (*ConfigSetDefaultString)(l_ConfigPlugins, "VideoPlugin", "mupen64plus-video-rice" OSAL_DLL_EXTENSION, "Filename of video plugin");
    (*ConfigSetDefaultString)(l_ConfigPlugins, "AudioPlugin", "mupen64plus-audio-sdl" OSAL_DLL_EXTENSION, "Filename of audio plugin");
    (*ConfigSetDefaultString)(l_ConfigPlugins, "InputPlugin", "mupen64plus-input-sdl" OSAL_DLL_EXTENSION, "Filename of input plugin");
    (*ConfigSetDefaultString)(l_ConfigPlugins, "RspPlugin", "mupen64plus-rsp-hle" OSAL_DLL_EXTENSION, "Filename of RSP plugin");

    return M64ERR_SUCCESS;
}

m64p_error SaveConfigurationOptions(void)
{
    /* if shared data directory was given on the command line, write it into the config file */
    if (l_DataDirPath != NULL)
        (*ConfigSetParameter)(l_ConfigCore, "SharedDataPath", M64TYPE_STRING, l_DataDirPath);

    /* if any plugin filepaths were given on the command line, write them into the config file */
    if (g_PluginDir != NULL)
        (*ConfigSetParameter)(l_ConfigPlugins, "PluginDir", M64TYPE_STRING, g_PluginDir);
    if (g_GfxPlugin != NULL)
        (*ConfigSetParameter)(l_ConfigPlugins, "VideoPlugin", M64TYPE_STRING, g_GfxPlugin);
    if (g_AudioPlugin != NULL)
        (*ConfigSetParameter)(l_ConfigPlugins, "AudioPlugin", M64TYPE_STRING, g_AudioPlugin);
    if (g_InputPlugin != NULL)
        (*ConfigSetParameter)(l_ConfigPlugins, "InputPlugin", M64TYPE_STRING, g_InputPlugin);
    if (g_RspPlugin != NULL)
        (*ConfigSetParameter)(l_ConfigPlugins, "RspPlugin", M64TYPE_STRING, g_RspPlugin);

    return (*ConfigSaveFile)();
}const char* getErrorMessage(m64p_error err){    return (*CoreErrorMessage)(err);}const char* getParameterHelp(m64p_handle* section, const char* ParamName){    return (*ConfigGetParameterHelp)(*section, ParamName);}m64p_error ReadConfigSectionParameters(const char* SectionName,                                        void (*ParameterListCallback)(void * sectionHandle,                                                                       const char *ParamName,                                                                       m64p_type ParamType)){    m64p_handle section;    m64p_error result = (*ConfigOpenSection)(SectionName, &section) ;    if (result != M64ERR_SUCCESS)    {        return result;    }    result = (*ConfigListParameters)(section, &section /* user data */, ParameterListCallback);    if (result != M64ERR_SUCCESS)    {        return result;    }    return M64ERR_SUCCESS;}m64p_error ReadConfigSections(void (*SectionListCallback)(void * context, const char * SectionName)){    m64p_error result = (*ConfigListSections)(NULL /* user data */, SectionListCallback);    if (result != M64ERR_SUCCESS)    {        return result;    }    return M64ERR_SUCCESS;}m64p_error getIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, int* valueOut){    int value = -1;    m64p_error result = (*ConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_INT, &value, sizeof(int));    if (result != M64ERR_SUCCESS)    {        return result;    }    *valueOut = value;    return M64ERR_SUCCESS;}m64p_error getBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, int* valueOut){    int value = -1;    m64p_error result = (*ConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_INT, &value, sizeof(int));    if (result != M64ERR_SUCCESS)    {        return result;    }    *valueOut = value;    return M64ERR_SUCCESS;}m64p_error getFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, float* valueOut){    float value = -1;    m64p_error result = (*ConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_FLOAT, &value, sizeof(float));    if (result != M64ERR_SUCCESS)    {        return result;    }    *valueOut = value;    return M64ERR_SUCCESS;}m64p_error getStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, char* valueOut, int maxSize){    m64p_error result = (*ConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_STRING, valueOut, maxSize);    if (result != M64ERR_SUCCESS)    {        return result;    }    return M64ERR_SUCCESS;}m64p_error setIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue){    m64p_error result = (*ConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_INT, &newValue);    if (result != M64ERR_SUCCESS)    {        return result;    }    return M64ERR_SUCCESS;}m64p_error setFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const float newValue){    m64p_error result = (*ConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_FLOAT, &newValue);    if (result != M64ERR_SUCCESS)    {        return result;    }    return M64ERR_SUCCESS;}m64p_error setBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue){    m64p_error result = (*ConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_BOOL, &newValue);    if (result != M64ERR_SUCCESS)    {        return result;    }    return M64ERR_SUCCESS;}m64p_error setStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const char* newValue){    m64p_error result = (*ConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_STRING, newValue);    if (result != M64ERR_SUCCESS)    {        return result;    }    return M64ERR_SUCCESS;}m64p_error createStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const char* newValue,                                   const char* ParamHelp){    return (*ConfigSetDefaultString)(ConfigSectionHandle, ParamName, newValue, ParamHelp);}m64p_error createIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue,                                const char* ParamHelp){    return (*ConfigSetDefaultInt)(ConfigSectionHandle, ParamName, newValue, ParamHelp);}m64p_error createFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const float newValue,                                 const char* ParamHelp){    return (*ConfigSetDefaultFloat)(ConfigSectionHandle, ParamName, newValue, ParamHelp);}m64p_error createBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue,                                 const char* ParamHelp){    return (*ConfigSetDefaultBool)(ConfigSectionHandle, ParamName, newValue, ParamHelp);}m64p_error InitCore(void){    return (*CoreStartup)(CONSOLE_API_VERSION, l_ConfigDirPath, l_DataDirPath, "Core",
                                      DebugCallback, NULL, NULL);}m64p_handle getSectionHandle(const char* sectionName){    m64p_handle handle;    m64p_error result = (*ConfigOpenSection)(sectionName, &handle);    assert (result == M64ERR_SUCCESS);    return handle;}m64p_error saveConfig(){    return (*ConfigSaveFile)();}