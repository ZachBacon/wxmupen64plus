/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus frontend                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Marianne Gagnon, based on work by Richard Goedeken *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "mupen64plusplus/MupenAPI.h"

#include "m64p_types.h"
#include "m64p_common.h"
#include "m64p_frontend.h"
#include "m64p_config.h"
#include "m64p_debugger.h"
#include "version.h"
#include "plugin.h"
#include "main.h"

#include "mupen64plusplus/osal_preproc.h"
#include "mupen64plusplus/osal_dynamiclib.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifndef NULL
#define NULL 0
#endif

/** static (local) variables **/
static m64p_handle l_ConfigCore = NULL;
static m64p_handle l_ConfigVideo = NULL;
static m64p_handle l_ConfigPlugins = NULL;

static const char *l_ConfigDirPath = NULL;

/* global data definitions */
int g_CoreCapabilities;

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
ptr_ConfigListSections     PtrConfigListSections = NULL;
ptr_ConfigOpenSection      PtrConfigOpenSection = NULL;
ptr_ConfigListParameters   PtrConfigListParameters = NULL;
ptr_ConfigSaveFile         PtrConfigSaveFile = NULL;
ptr_ConfigSetParameter     PtrConfigSetParameter = NULL;
ptr_ConfigGetParameter     PtrConfigGetParameter = NULL;
ptr_ConfigGetParameterType PtrConfigGetParameterType = NULL;
ptr_ConfigGetParameterHelp PtrConfigGetParameterHelp = NULL;
ptr_ConfigSetDefaultInt    PtrConfigSetDefaultInt = NULL;
ptr_ConfigSetDefaultFloat  PtrConfigSetDefaultFloat = NULL;
ptr_ConfigSetDefaultBool   PtrConfigSetDefaultBool = NULL;
ptr_ConfigSetDefaultString PtrConfigSetDefaultString = NULL;
ptr_ConfigGetParamInt      PtrConfigGetParamInt = NULL;
ptr_ConfigGetParamFloat    PtrConfigGetParamFloat = NULL;
ptr_ConfigGetParamBool     PtrConfigGetParamBool = NULL;
ptr_ConfigGetParamString   PtrConfigGetParamString = NULL;

ptr_ConfigGetSharedDataFilepath PtrConfigGetSharedDataFilepath = NULL;
ptr_ConfigGetUserConfigPath     PtrConfigGetUserConfigPath = NULL;
ptr_ConfigGetUserDataPath       PtrConfigGetUserDataPath = NULL;
ptr_ConfigGetUserCachePath      PtrConfigGetUserCachePath = NULL;

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

m64p_dynlib_handle CoreHandle = NULL;

// -----------------------------------------------------------------------------------------------------------

char concat_buff[512];
const char* concat(const char* a, const char* b)
{
    strcpy(concat_buff, a);
    strcat(concat_buff, b);
    return concat_buff;
}


// -----------------------------------------------------------------------------------------------------------
// FIXME: copied from mupen64plus-core/src/osd/osd.h
enum osd_corner
{
    OSD_TOP_LEFT,       // 0 in the picture above
    OSD_TOP_CENTER,     // 1 in the picture above
    OSD_TOP_RIGHT,      // 2 in the picture above

    OSD_MIDDLE_LEFT,    // 3 in the picture above
    OSD_MIDDLE_CENTER,  // 4 in the picture above
    OSD_MIDDLE_RIGHT,   // 5 in the picture above

    OSD_BOTTOM_LEFT,    // 6 in the picture above
    OSD_BOTTOM_CENTER,  // 7 in the picture above
    OSD_BOTTOM_RIGHT,   // 8 in the picture above

    OSD_NUM_CORNERS
};
enum osd_message_state
{
    OSD_APPEAR,     // OSD message is appearing on the screen
    OSD_DISPLAY,    // OSD message is being displayed on the screen
    OSD_DISAPPEAR,  // OSD message is disappearing from the screen

    OSD_NUM_STATES
};
enum osd_animation_type
{
    OSD_NONE,
    OSD_FADE,

    OSD_NUM_ANIM_TYPES
};
typedef struct
{
    char *text;        // Text that this object will have when displayed
    enum osd_corner corner; // One of the 9 corners
    float xoffset;     // Relative X position
    float yoffset;     // Relative Y position
    float color[3];    // Red, Green, Blue values
    float sizebox[4];  // bounding box (xmin, ymin, xmax, ymax)
    int state;         // display state of current message
    enum osd_animation_type animation[OSD_NUM_STATES]; // animations for each display state
    unsigned int timeout[OSD_NUM_STATES]; // timeouts for each display state
#define OSD_INFINITE_TIMEOUT 0xffffffff
    unsigned int frames; // number of frames in this state
} osd_message_t;

typedef osd_message_t* (*ptr_osdMessage)(enum osd_corner, const char *, ...);
ptr_osdMessage osd_new_message = NULL;

// -----------------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------------------

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
        mplog_error("AttachCoreLib", "ERROR: failed to find Mupen64Plus Core library\n");
        CoreHandle = NULL;
        return M64ERR_INPUT_NOT_FOUND;
    }

    // attach and call the PluginGetVersion function, check the Core and API versions
    // for compatibility with this front-end
    ptr_PluginGetVersion CoreVersionFunc;
    CoreVersionFunc = (ptr_PluginGetVersion) osal_dynlib_getproc(CoreHandle, "PluginGetVersion");
    if (CoreVersionFunc == NULL)
    {
        mplog_error("AttachCoreLib", "ERROR: Shared library '%s' invalid; no "
                    "PluginGetVersion() function found.\n", CoreLibFilepath);
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
    {
        mplog_error("AttachCoreLib", "ERROR: Shared library '%s' invalid; this is not the emulator core.\n",
                    CoreLibFilepath);
    }
    else if (CoreVersion < MINIMUM_CORE_VERSION)
    {
        mplog_error("AttachCoreLib", "ERROR: Shared library '%s' incompatible; core version %i.%i.%i "
                    "is below minimum supported %i.%i.%i\n", CoreLibFilepath,
                    VERSION_PRINTF_SPLIT(CoreVersion),
                    VERSION_PRINTF_SPLIT(MINIMUM_CORE_VERSION));
    }
    else if ((APIVersion & 0xffff0000) != (CORE_API_VERSION & 0xffff0000))
    {
        fprintf(stderr, "AttachCoreLib() Error: Shared library '%s' incompatible; core API major version "
                        "%i.%i.%i doesn't match with this application (%i.%i.%i)\n", CoreLibFilepath,
                        VERSION_PRINTF_SPLIT(APIVersion), VERSION_PRINTF_SPLIT(CORE_API_VERSION));
    }
    else
    {
        Compatible = 1;
    }

    /* exit if not compatible */
    if (Compatible == 0)
    {
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INCOMPATIBLE;
    }

    /* attach and call the CoreGetAPIVersion function, check Config API version for compatibility */
    ptr_CoreGetAPIVersions CoreAPIVersionFunc;
    CoreAPIVersionFunc = (ptr_CoreGetAPIVersions) osal_dynlib_getproc(CoreHandle, "CoreGetAPIVersions");
    if (CoreAPIVersionFunc == NULL)
    {
        fprintf(stderr, "AttachCoreLib() Error: Library '%s' broken; no CoreAPIVersionFunc() function found.\n",
                        CoreLibFilepath);
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INPUT_INVALID;
    }

    int ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion;
    (*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);

    if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000))
    {
        fprintf(stderr, "AttachCoreLib() Error: Emulator core '%s' incompatible; Config API major version "
                        "%i.%i.%i doesn't match application: %i.%i.%i\n", CoreLibFilepath,
                        VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INCOMPATIBLE;
    }

    // print some information about the core library
    mplog_info("MupenAPI", "attached to core library '%s' version %i.%i.%i\n", CoreName, VERSION_PRINTF_SPLIT(CoreVersion));
    if (g_CoreCapabilities & M64CAPS_DYNAREC)
        mplog_info("MupenAPI", "            Includes support for Dynamic Recompiler.\n");
    if (g_CoreCapabilities & M64CAPS_DEBUGGER)
        mplog_info("MupenAPI", "            Includes support for MIPS r4300 Debugger.\n");
    if (g_CoreCapabilities & M64CAPS_CORE_COMPARE)
        mplog_info("MupenAPI", "            Includes support for r4300 Core Comparison.\n");

    // get function pointers to the common and front-end functions
    CoreErrorMessage   = (ptr_CoreErrorMessage) osal_dynlib_getproc(CoreHandle, "CoreErrorMessage");
    CoreStartup        = (ptr_CoreStartup) osal_dynlib_getproc(CoreHandle, "CoreStartup");
    CoreShutdown       = (ptr_CoreShutdown) osal_dynlib_getproc(CoreHandle, "CoreShutdown");
    CoreAttachPlugin   = (ptr_CoreAttachPlugin) osal_dynlib_getproc(CoreHandle, "CoreAttachPlugin");
    CoreDetachPlugin   = (ptr_CoreDetachPlugin) osal_dynlib_getproc(CoreHandle, "CoreDetachPlugin");
    CoreDoCommand      = (ptr_CoreDoCommand) osal_dynlib_getproc(CoreHandle, "CoreDoCommand");
    CoreOverrideVidExt = (ptr_CoreOverrideVidExt) osal_dynlib_getproc(CoreHandle, "CoreOverrideVidExt");
    CoreAddCheat       = (ptr_CoreAddCheat) osal_dynlib_getproc(CoreHandle, "CoreAddCheat");
    CoreCheatEnabled   = (ptr_CoreCheatEnabled) osal_dynlib_getproc(CoreHandle, "CoreCheatEnabled");

    // get function pointers to the configuration functions
    PtrConfigListSections     = (ptr_ConfigListSections)     osal_dynlib_getproc(CoreHandle, "ConfigListSections");
    PtrConfigOpenSection      = (ptr_ConfigOpenSection)      osal_dynlib_getproc(CoreHandle, "ConfigOpenSection");
    PtrConfigListParameters   = (ptr_ConfigListParameters)   osal_dynlib_getproc(CoreHandle, "ConfigListParameters");
    PtrConfigSaveFile         = (ptr_ConfigSaveFile)         osal_dynlib_getproc(CoreHandle, "ConfigSaveFile");
    PtrConfigSetParameter     = (ptr_ConfigSetParameter)     osal_dynlib_getproc(CoreHandle, "ConfigSetParameter");
    PtrConfigGetParameter     = (ptr_ConfigGetParameter)     osal_dynlib_getproc(CoreHandle, "ConfigGetParameter");
    PtrConfigGetParameterType = (ptr_ConfigGetParameterType) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterType");
    PtrConfigGetParameterHelp = (ptr_ConfigGetParameterHelp) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterHelp");
    PtrConfigSetDefaultInt    = (ptr_ConfigSetDefaultInt)    osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultInt");
    PtrConfigSetDefaultFloat  = (ptr_ConfigSetDefaultFloat)  osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultFloat");
    PtrConfigSetDefaultBool   = (ptr_ConfigSetDefaultBool)   osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultBool");
    PtrConfigSetDefaultString = (ptr_ConfigSetDefaultString) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultString");
    PtrConfigGetParamInt      = (ptr_ConfigGetParamInt)      osal_dynlib_getproc(CoreHandle, "ConfigGetParamInt");
    PtrConfigGetParamFloat    = (ptr_ConfigGetParamFloat)    osal_dynlib_getproc(CoreHandle, "ConfigGetParamFloat");
    PtrConfigGetParamBool     = (ptr_ConfigGetParamBool)     osal_dynlib_getproc(CoreHandle, "ConfigGetParamBool");
    PtrConfigGetParamString   = (ptr_ConfigGetParamString)   osal_dynlib_getproc(CoreHandle, "ConfigGetParamString");

    PtrConfigGetSharedDataFilepath = (ptr_ConfigGetSharedDataFilepath) osal_dynlib_getproc(CoreHandle, "ConfigGetSharedDataFilepath");
    PtrConfigGetUserConfigPath     = (ptr_ConfigGetUserConfigPath)     osal_dynlib_getproc(CoreHandle, "ConfigGetUserConfigPath");
    PtrConfigGetUserDataPath       = (ptr_ConfigGetUserDataPath)       osal_dynlib_getproc(CoreHandle, "ConfigGetUserDataPath");
    PtrConfigGetUserCachePath      = (ptr_ConfigGetUserCachePath)      osal_dynlib_getproc(CoreHandle, "ConfigGetUserCachePath");

    // get function pointers to the debugger functions
    DebugSetCallbacks     = (ptr_DebugSetCallbacks)     osal_dynlib_getproc(CoreHandle, "DebugSetCallbacks");
    DebugSetCoreCompare   = (ptr_DebugSetCoreCompare)   osal_dynlib_getproc(CoreHandle, "DebugSetCoreCompare");
    DebugSetRunState      = (ptr_DebugSetRunState)      osal_dynlib_getproc(CoreHandle, "DebugSetRunState");
    DebugGetState         = (ptr_DebugGetState)         osal_dynlib_getproc(CoreHandle, "DebugGetState");
    DebugStep             = (ptr_DebugStep)             osal_dynlib_getproc(CoreHandle, "DebugStep");
    DebugDecodeOp         = (ptr_DebugDecodeOp)         osal_dynlib_getproc(CoreHandle, "DebugDecodeOp");
    DebugMemGetRecompInfo = (ptr_DebugMemGetRecompInfo) osal_dynlib_getproc(CoreHandle, "DebugMemGetRecompInfo");
    DebugMemGetMemInfo    = (ptr_DebugMemGetMemInfo)    osal_dynlib_getproc(CoreHandle, "DebugMemGetMemInfo");
    DebugMemGetPointer    = (ptr_DebugMemGetPointer)    osal_dynlib_getproc(CoreHandle, "DebugMemGetPointer");

    DebugMemRead64 = (ptr_DebugMemRead64) osal_dynlib_getproc(CoreHandle, "DebugMemRead64");
    DebugMemRead32 = (ptr_DebugMemRead32) osal_dynlib_getproc(CoreHandle, "DebugMemRead32");
    DebugMemRead16 = (ptr_DebugMemRead16) osal_dynlib_getproc(CoreHandle, "DebugMemRead16");
    DebugMemRead8  = (ptr_DebugMemRead8)   osal_dynlib_getproc(CoreHandle, "DebugMemRead8");

    DebugMemWrite64 = (ptr_DebugMemWrite64) osal_dynlib_getproc(CoreHandle, "DebugMemWrite64");
    DebugMemWrite32 = (ptr_DebugMemWrite32) osal_dynlib_getproc(CoreHandle, "DebugMemWrite32");
    DebugMemWrite16 = (ptr_DebugMemWrite16) osal_dynlib_getproc(CoreHandle, "DebugMemWrite16");
    DebugMemWrite8  = (ptr_DebugMemWrite8)  osal_dynlib_getproc(CoreHandle, "DebugMemWrite8");

    DebugGetCPUDataPtr     = (ptr_DebugGetCPUDataPtr)     osal_dynlib_getproc(CoreHandle, "DebugGetCPUDataPtr");
    DebugBreakpointLookup  = (ptr_DebugBreakpointLookup)  osal_dynlib_getproc(CoreHandle, "DebugBreakpointLookup");
    DebugBreakpointCommand = (ptr_DebugBreakpointCommand) osal_dynlib_getproc(CoreHandle, "DebugBreakpointCommand");

    osd_new_message = (ptr_osdMessage) osal_dynlib_getproc(CoreHandle, "osd_new_message");

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error DetachCoreLib(void)
{
    if (CoreHandle == NULL)
        return M64ERR_INVALID_STATE;

    // set the core function pointers to NULL
    CoreErrorMessage = NULL;
    CoreStartup = NULL;
    CoreShutdown = NULL;
    CoreAttachPlugin = NULL;
    CoreDetachPlugin = NULL;
    CoreDoCommand = NULL;
    CoreOverrideVidExt = NULL;
    CoreAddCheat = NULL;
    CoreCheatEnabled = NULL;

    PtrConfigListSections = NULL;
    PtrConfigOpenSection = NULL;
    PtrConfigListParameters = NULL;
    PtrConfigSetParameter = NULL;
    PtrConfigGetParameter = NULL;
    PtrConfigGetParameterType = NULL;
    PtrConfigGetParameterHelp = NULL;
    PtrConfigSetDefaultInt = NULL;
    PtrConfigSetDefaultBool = NULL;
    PtrConfigSetDefaultString = NULL;
    PtrConfigGetParamInt = NULL;
    PtrConfigGetParamBool = NULL;
    PtrConfigGetParamString = NULL;

    PtrConfigGetSharedDataFilepath = NULL;
    PtrConfigGetUserDataPath = NULL;
    PtrConfigGetUserCachePath = NULL;

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

    // detach the shared library
    osal_dynlib_close(CoreHandle);
    CoreHandle = NULL;

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_handle getConfigUI()
{
    assert(l_ConfigPlugins != NULL);
    return l_ConfigPlugins;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error OpenConfigurationHandles(const char* defaultPluginDir,
                                    const char* defaultVideoPlugin, const char* defaultAudioPlugin,
                                    const char* defaultInputPlugin, const char* defaultRspPlugin)
{
    m64p_error rval;

    // Open Configuration sections for core library and console User Interface
    rval = (*PtrConfigOpenSection)("Core", &l_ConfigCore);
    if (rval != M64ERR_SUCCESS)
    {
        mplog_warning("Config", "WARNING: failed to open 'Core' configuration section\n");
        return rval;
    }

    rval = (*PtrConfigOpenSection)("Video-General", &l_ConfigVideo);
    if (rval != M64ERR_SUCCESS)
    {
        mplog_warning("Config", "WARNING: failed to open 'Video-General' configuration section\n");
        return rval;
    }

    rval = (*PtrConfigOpenSection)("UI-wx", &l_ConfigPlugins);
    if (rval != M64ERR_SUCCESS)
    {
        mplog_warning("Config", "WARNING: failed to open 'UI-wx' configuration section\n");
        return rval;
    }


    // Set default values for my Config parameters
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "PluginDir", defaultPluginDir,
                                 "Directory in which to search for plugins");
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "VideoPlugin",
                                 concat(defaultVideoPlugin, OSAL_DLL_EXTENSION),
                                 "Filename of video plugin");
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "AudioPlugin",
                                 concat(defaultAudioPlugin, OSAL_DLL_EXTENSION),
                                 "Filename of audio plugin");
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "InputPlugin",
                                 concat(defaultInputPlugin, OSAL_DLL_EXTENSION),
                                 "Filename of input plugin");
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "RspPlugin",
                                 concat(defaultRspPlugin, OSAL_DLL_EXTENSION),
                                 "Filename of RSP plugin");
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "GamesPath", "",
                                 "Where to search for games");
#ifdef __WXGTK__
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "VideoExtension", "external",
                                 "Whether to run game in main frame or in external frame. Running in external frame works better on Linux.");
#else
    (*PtrConfigSetDefaultString)(l_ConfigPlugins, "VideoExtension", "internal",
                                     "Whether to run game in main frame or in external frame. Running in external frame works better on Linux.");
#endif
    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error GetConfigPlugins(char pluginsPath[], const int pluginsPathLen,
                            char videoPlugin[], const int videoPluginLen,
                            char audioPlugin[], const int audioPluginLen,
                            char inputPlugin[], const int inputPluginLen,
                            char rspPlugin[],   const int rspPluginLen)
{
    m64p_error rval = getStringConfigParam(l_ConfigPlugins, "PluginDir", pluginsPath, pluginsPathLen);
    if (rval != M64ERR_SUCCESS) return rval;

    rval = getStringConfigParam(l_ConfigPlugins, "VideoPlugin", videoPlugin, videoPluginLen);
    if (rval != M64ERR_SUCCESS) return rval;

    rval = getStringConfigParam(l_ConfigPlugins, "AudioPlugin", audioPlugin, audioPluginLen);
    if (rval != M64ERR_SUCCESS) return rval;

    rval = getStringConfigParam(l_ConfigPlugins, "InputPlugin", inputPlugin, inputPluginLen);
    if (rval != M64ERR_SUCCESS) return rval;

    rval = getStringConfigParam(l_ConfigPlugins, "RspPlugin", rspPlugin, rspPluginLen);
    if (rval != M64ERR_SUCCESS) return rval;

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error GetVideoExtension(char videoExtension[], const int videoExtensionLen)
{
    return getStringConfigParam(l_ConfigPlugins, "VideoExtension",
                                videoExtension, videoExtensionLen);
}

// -----------------------------------------------------------------------------------------------------------

const char* getErrorMessage(m64p_error err)
{
    static char buffer[1024];
    snprintf(buffer, 1024, "(%i) %s", (int)err, (*CoreErrorMessage)(err));
    return buffer;
}

// -----------------------------------------------------------------------------------------------------------

const char* getParameterHelp(m64p_handle* section, const char* ParamName)
{
    return (*PtrConfigGetParameterHelp)(*section, ParamName);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error ReadConfigSectionParameters(const char* SectionName,
                                        void (*ParameterListCallback)(void * sectionHandle,
                                                                      const char *ParamName,
                                                                      m64p_type ParamType))
{
    m64p_handle section;

    m64p_error result = (*PtrConfigOpenSection)(SectionName, &section) ;
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    result = (*PtrConfigListParameters)(section, &section /* user data */, ParameterListCallback);
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error ReadConfigSections(void (*SectionListCallback)(void * context, const char * SectionName),
                              void* userdata)
{
    m64p_error result = (*PtrConfigListSections)(userdata, SectionListCallback);
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, int* valueOut)
{
    int value = -1;
    m64p_error result = (*PtrConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_INT,
                                                 &value, sizeof(int));
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    *valueOut = value;
    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, int* valueOut)
{
    int value = -1;
    m64p_error result = (*PtrConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_BOOL,
                                                 &value, sizeof(int));
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    *valueOut = value;
    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, float* valueOut)
{
    float value = -1;
    m64p_error result = (*PtrConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_FLOAT,
                                                 &value, sizeof(float));
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    *valueOut = value;
    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName,
                                char* valueOut, int maxSize)
{
    m64p_error result = (*PtrConfigGetParameter)(ConfigSectionHandle, ParamName, M64TYPE_STRING,
                                                 valueOut, maxSize);
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error setIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue)
{
    m64p_error result = (*PtrConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_INT, &newValue);
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error setFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const float newValue)
{
    m64p_error result = (*PtrConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_FLOAT, &newValue);
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error setBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue)
{
    m64p_error result = (*PtrConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_BOOL, &newValue);
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error setStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const char* newValue)
{
    m64p_error result = (*PtrConfigSetParameter)(ConfigSectionHandle, ParamName, M64TYPE_STRING, newValue);
    if (result != M64ERR_SUCCESS)
    {
        return result;
    }

    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error createStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName,
                                   const char* newValue, const char* ParamHelp)
{
    return (*PtrConfigSetDefaultString)(ConfigSectionHandle, ParamName, newValue, ParamHelp);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error createIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue,
                                const char* ParamHelp)
{
    return (*PtrConfigSetDefaultInt)(ConfigSectionHandle, ParamName, newValue, ParamHelp);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error createFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName,
                                  const float newValue, const char* ParamHelp)
{
    return (*PtrConfigSetDefaultFloat)(ConfigSectionHandle, ParamName, newValue, ParamHelp);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error createBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue,
                                 const char* ParamHelp)
{
    return (*PtrConfigSetDefaultBool)(ConfigSectionHandle, ParamName, newValue, ParamHelp);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error InitCore(ptr_StateCallback stateCallback, void* context, const char* dataPath)
{
    return (*CoreStartup)(CORE_API_VERSION, l_ConfigDirPath, dataPath,
                          (void*)("Core") /* context */, DebugCallback,
                          context, stateCallback);
}

// -----------------------------------------------------------------------------------------------------------

m64p_handle getSectionHandle(const char* sectionName)
{
    m64p_handle handle;
    m64p_error result = (*PtrConfigOpenSection)(sectionName, &handle);

    assert (result == M64ERR_SUCCESS);

    return handle;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error saveConfig()
{
    return (*PtrConfigSaveFile)();
}

// -----------------------------------------------------------------------------------------------------------

m64p_error openRom(int byteSize, void* romImage)
{
    return (*CoreDoCommand)(M64CMD_ROM_OPEN, byteSize, romImage);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error closeRom()
{
    return (*CoreDoCommand)(M64CMD_ROM_CLOSE, 0 /* unused */, NULL /* unused */);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getCurrentRomHeader(m64p_rom_header* out)
{
    return (*CoreDoCommand)(M64CMD_ROM_GET_HEADER, sizeof(m64p_rom_header), out);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getRomHeader(const char* path, m64p_rom_header* out)
{
    FILE* f;
    f = fopen(path, "rb");

    if (!f) return M64ERR_FILES;

    int count = fread(out, sizeof(m64p_rom_header), 1, f);

    int error = (count != 1) || ferror(f);

    fclose(f);

    if (error != 0) return M64ERR_NO_MEMORY;
    else            return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getRomSettings(m64p_rom_settings* out)
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_ROM_GET_SETTINGS, sizeof(m64p_rom_settings), out);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error runEmulation()
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_EXECUTE, 0 /* unused */, NULL /* unused */);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error stopEmulation()
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_STOP, 0 /* unused */, NULL /* unused */);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error pauseEmulation()
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_PAUSE, 0 /* unused */, NULL /* unused */);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error resumeEmulation()
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_RESUME, 0 /* unused */, NULL /* unused */);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error getState(m64p_core_param  which, int* out)
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_CORE_STATE_QUERY, which, out );
}

// -----------------------------------------------------------------------------------------------------------

m64p_error saveGame(int pj64Format, char* path /* optional */)
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_STATE_SAVE, (pj64Format ? 2 : 1), path);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error setSaveSlot(int slotId)
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_STATE_SET_SLOT, slotId, NULL);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error takeScreenshot()
{
    assert(CoreDoCommand != NULL);
    return (*CoreDoCommand)(M64CMD_TAKE_NEXT_SCREENSHOT, 0 /* unused */, NULL /* unused */);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error attachPlugins()
{
    int i;

    // attach plugins to core
    for (i = 0; i < 4; i++)
    {
        mplog_info("MupenAPI", "Attaching plugin %i of type %i : %s\n", i, g_PluginMap[i].type, g_PluginMap[i].name);
        m64p_error result = (*CoreAttachPlugin)(g_PluginMap[i].type, g_PluginMap[i].handle);
        if (result != M64ERR_SUCCESS)
        {
            return result;
        }
    }
    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error detachPlugins()
{
    assert(CoreDetachPlugin != NULL);
    int i;

    // detach plugins from core and unload them
    for (i = 0; i < 4; i++)
    {
        (*CoreDetachPlugin)(g_PluginMap[i].type);
    }
    return M64ERR_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------

m64p_error coreOverrideVidExt(m64p_video_extension_functions* VideoFunctionStruct)
{
    return (*CoreOverrideVidExt)(VideoFunctionStruct);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error injectKeyEvent(int /* bool */ pressed, int key)
{
    if (pressed) return (*CoreDoCommand)(M64CMD_SEND_SDL_KEYDOWN, key, NULL);
    else         return (*CoreDoCommand)(M64CMD_SEND_SDL_KEYUP,   key, NULL);
}

// -----------------------------------------------------------------------------------------------------------

void osdNewMessage(const char* message)
{
    (*osd_new_message)(OSD_BOTTOM_CENTER, message);
}

// -----------------------------------------------------------------------------------------------------------

const char *GetUserConfigPath()
{
    return (*PtrConfigGetUserConfigPath)();
}

// -----------------------------------------------------------------------------------------------------------

const char *GetUserDataPath()
{
    return (*PtrConfigGetUserDataPath)();
}

// -----------------------------------------------------------------------------------------------------------

m64p_error SetDebuggingCallbacks(void (*init)(void), void (*update)(unsigned int), void (*vi)(void))
{
    return (*DebugSetCallbacks)(init, update, vi);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error SetRunState(int state)
{
    return (*DebugSetRunState)(state);
}

// -----------------------------------------------------------------------------------------------------------

m64p_error DebuggerStep()
{
    return (*DebugStep)();
}

// -----------------------------------------------------------------------------------------------------------

int GetDebugState(m64p_dbg_state state)
{
    return (*DebugGetState)(state);
}

// -----------------------------------------------------------------------------------------------------------

int MemIsValid(unsigned int address)
{
    int flags = (*DebugMemGetMemInfo)(M64P_DBG_MEM_FLAGS, address);
    if (flags & 0x1 || flags & 0x2) // The api headers were lacking
        return 1;

    // io addresses aren't included in the previous check
    if (address >= 0xa0000000)
        address -= 0x20000000;
    if ((address >= 0x83f00000 && address < 0x8480001c) &&
        ((address >= 0x83f00000 && address < 0x83f00028) ||
         (address >= 0x84000000 && address < 0x84002000) ||
         (address >= 0x84040000 && address < 0x84040020) ||
         (address >= 0x84080000 && address < 0x84080008) ||
         (address >= 0x84100000 && address < 0x84100020) ||
         (address >= 0x84200000 && address < 0x84200010) ||
         (address >= 0x84300000 && address < 0x84300010) ||
         (address >= 0x84400000 && address < 0x84400038) ||
         (address >= 0x84500000 && address < 0x84500018) ||
         (address >= 0x84600000 && address < 0x84600034) ||
         (address >= 0x84700000 && address < 0x84700020) ||
         (address >= 0x84800000 && address < 0x8480001c)))
        return 1;

    return 0;
}

// -----------------------------------------------------------------------------------------------------------

unsigned char MemRead8(unsigned int address)
{
    return (*DebugMemRead8)(address);
}

// -----------------------------------------------------------------------------------------------------------

unsigned short MemRead16(unsigned int address)
{
    return (*DebugMemRead16)(address);
}

// -----------------------------------------------------------------------------------------------------------

unsigned int MemRead32(unsigned int address)
{
    return (*DebugMemRead32)(address);
}

// -----------------------------------------------------------------------------------------------------------

unsigned long long MemRead64(unsigned int address)
{
    return (*DebugMemRead64)(address);
}

// -----------------------------------------------------------------------------------------------------------

void MemWrite8(unsigned int address, unsigned char value)
{
    (*DebugMemWrite8)(address, value);
}

// -----------------------------------------------------------------------------------------------------------

void MemWrite32(unsigned int address, unsigned int value)
{
    (*DebugMemWrite32)(address, value);
}

// -----------------------------------------------------------------------------------------------------------

void *GetRegister(m64p_dbg_cpu_data type)
{
    return (*DebugGetCPUDataPtr)(type);
}

// -----------------------------------------------------------------------------------------------------------

unsigned long long ReadRegister(m64p_dbg_cpu_data type, unsigned char index)
{
    switch (type)
    {
        case M64P_CPU_REG_REG:
        case M64P_CPU_REG_COP1_FGR_64:
            return ((unsigned long long *)GetRegister(type))[index];
        default:
            return ((unsigned long *)GetRegister(type))[index];
        break;
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------

void *GetMemoryPointer(m64p_dbg_memptr_type type)
{
    return (*DebugMemGetPointer)(type);
}

// -----------------------------------------------------------------------------------------------------------

void DecodeOpcode(unsigned int addr, char *op, char *args)
{
    (*DebugDecodeOp)(MemRead32(addr), op, args, addr);
}

