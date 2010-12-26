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

#ifndef __MUPEN64_API__
#define __MUPEN64_API__

#ifdef __cplusplus
extern "C" {
#endif

//#include "callbacks.h"
//#include "config.h"
#include "m64p_config.h"
#include "m64p_frontend.h"
#include "m64p_common.h"
#include "m64p_plugin.h"

extern m64p_dynlib_handle CoreHandle;
extern ptr_ConfigGetParamString   PtrConfigGetParamString;

extern void DebugCallback(void *Context, int level, const char *message);

m64p_error AttachCoreLib(const char *CoreLibFilepath);
m64p_error DetachCoreLib(void);
m64p_error InitCore(ptr_StateCallback stateCallback, void* context);

extern m64p_error OpenConfigurationHandles(const char* defaultPluginDir,
                                           const char* defaultVideoPlugin, const char* defaultAudioPlugin,
                                           const char* defaultInputPlugin, const char* defaultRspPlugin);
extern m64p_error SaveConfigurationOptions(void);
m64p_error ReadConfigSections(void (*SectionListCallback)(void * context, const char * SectionName));
m64p_error ReadConfigSectionParameters(const char* section, void (*ParameterListCallback)(void * sectionHandle,
                                                                       const char *ParamName,
                                                                       m64p_type ParamType));
const char* getParameterHelp(m64p_handle* section, const char* paramName);

const char* getErrorMessage(m64p_error err);

m64p_handle getConfigUI();
m64p_error getIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, int* valueOut);
m64p_error getBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, int* valueOut);
m64p_error getFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, float* valueOut);
m64p_error getStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, char* valueOut, int maxSize);

m64p_error setStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const char* newValue);
m64p_error setIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue);
m64p_error setFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const float newValue);
m64p_error setBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue);

m64p_error createStringConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const char* newValue, const char* help);
m64p_error createIntConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue, const char* help);
m64p_error createFloatConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const float newValue, const char* help);
m64p_error createBoolConfigParam(m64p_handle ConfigSectionHandle, const char *ParamName, const int newValue, const char* help);

m64p_handle getSectionHandle(const char* sectionName);

m64p_error saveConfig();

m64p_error GetConfigPlugins(char pluginsPath[], const int pluginsPathLen,
                            char videoPlugin[], const int videoPluginLen,
                            char audioPlugin[], const int audioPluginLen,
                            char inputPlugin[], const int inputPluginLen,
                            char rspPlugin[],   const int rspPluginLen);

extern ptr_CoreShutdown  CoreShutdown;
extern ptr_CoreStartup   CoreStartup;

/**
 * @pre The emulator cannot be currently running. A ROM image must not be currently opened
 */
m64p_error openRom(int byteSize, void* romImage);

/**
 * @pre The emulator cannot be currently running. A ROM image must have been previously opened.
 *      There should be no plugins currently attached. 
 */
m64p_error closeRom();

/**
 * @pre The emulator cannot be currently running. A ROM image must have been previously opened.
 * @note This function call will not return until the game has been stopped. 
 */
m64p_error runEmulation();

/**
 * This will stop the emulator, if it is currently running.
 * @note This command will execute asynchronously. 
 */
m64p_error stopEmulation();

/**
 * This command will pause the emulator if it is running.
 * @return This function will return a non-successful error code if the emulator is in the stopped state.
 * @note This command may execute asynchronously. 
 */
m64p_error pauseEmulation();

/**
 * This command will resume execution of the emulator if it is paused. 
 * @return This function will return a non-successful error code if the emulator is in the stopped state.
 * @note This command may execute asynchronously. 
 */
m64p_error resumeEmulation();

m64p_error attachPlugins();
m64p_error detachPlugins();

/**
 * Get information about currently loaded rom
 * @pre A ROM must be loaded
 */
m64p_error getCurrentRomHeader(m64p_rom_header* out);

/**
 * Get information about the given ROM
 * @pre A ROM must be loaded
 */
m64p_error getRomHeader(const char* path, m64p_rom_header* out);

m64p_error getRomSettings(m64p_rom_settings* out);

/** 
 * @param pj64Format boolean
 */
m64p_error saveGame(int pj64Format, char* path /* optional */);

m64p_error getState(m64p_core_param  which, int* out);

/**
 * @param slotID Value to set for the current slot index. Must be between 0 and 9
 */
m64p_error setSaveSlot(int slotId);

/**
 * This will cause the core to save a screenshot at the next possible opportunity. 
 * The emulator must be currently running or paused. This command will execute asynchronously.
 */
m64p_error takeScreenshot();


/** Get the path where to find games, in config */
//m64p_error GetGamesPath(char path[], const int pathLen);

/** Set the path where to find games, in config */
//m64p_error SetGamesPath(char path[]);

#ifdef __cplusplus
}
#endif

#endif
