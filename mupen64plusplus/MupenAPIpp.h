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

#ifndef __MUPEN_API_PP__
#define __MUPEN_API_PP__

#include <vector>
#include <string>
#include "m64p_types.h"
#include <wx/variant.h>

class wxProgressDialog;

class ConfigParamChoice
{
public:
    wxString m_name;
    int m_value;

    ConfigParamChoice(wxString name, int value)
    {
        m_name = name;
        m_value = value;
    }
};

/** Enum used to give a special type to a param so that it is displayed in a way that is more user-friendly
  * considered what this parameter is
  */
enum SpecialParamType
{
    NOTHING_SPECIAL,

    /** A keyboard kehy binding, stored using integer format in config */
    KEYBOARD_KEY_INT,

    /** A key binding, stored using a string like "key(111)" or "button(9)" or "axis(3+)" or "hat(0 Right)"
     *  format in config */
    BINDING_DIGITAL_STRING,
    
    /** A key binding, stored using a format like "key(111, 222)" or "axis(0-,0+)" in config */
    BINDING_ANALOG_COUPLE_STRING,
    
    /** A directory parameter (primitive type = string)*/
    DIRECTORY,
    
    /** Name of a plugin dll */
    PLUGIN_FILE
};

class ConfigParam
{
    unsigned long m_magic_number;

public:

    m64p_type   m_param_type;
    std::string m_param_name;
    std::string m_help_string;
    m64p_handle m_parent_section;
    bool        m_enabled;
    
    /** Not read for all types of parameters. Second icon used only for parameter couples */
    wxString m_icon_1;
    wxString m_icon_2;

    SpecialParamType m_special_type;

    /** will be non-empty if some pre-defined choices are available */
    std::vector<ConfigParamChoice> m_choices;

    /** Only set if m_special_type is PLUGIN_FILE. This contains the parameter that holds the
     *  directory to search */
    ConfigParam* m_dir;

    /** Dummy instance ctor; produces a non-usable instance that needs to be setup later */
    ConfigParam()
    {
        m_enabled = false;
        m_parent_section = 0;
        m_special_type = NOTHING_SPECIAL;
        m_magic_number = 0xC001C001;
        m_dir = NULL;
    }

    ConfigParam(m64p_handle section, SpecialParamType type = NOTHING_SPECIAL)
    {
        m_enabled = true;
        m_parent_section = section;
        m_special_type = type;
        m_magic_number = 0xC001C001;
        m_dir = NULL;
    }

    ConfigParam(const ConfigParam& other)
    {
        m_magic_number = 0xC001C001;
        m_enabled = other.m_enabled;
        m_parent_section = other.m_parent_section;
        m_special_type = other.m_special_type;
        m_choices = other.m_choices;
        m_param_name = other.m_param_name;
        m_param_type = other.m_param_type;
        m_help_string = other.m_help_string;
        m_icon_1 = other.m_icon_1;
        m_icon_2 = other.m_icon_2;
        
        if (other.m_dir != NULL)
        {
            m_dir = new ConfigParam(*other.m_dir);
        }
        else
        {
            m_dir = NULL;
        }
    }

    ~ConfigParam()
    {
        m_magic_number = 0xDEADBEEF;
        delete m_dir;
    }

    bool ok() const { return (m_magic_number == 0xC001C001); }

    /** @return whether the current parameter and the one given as parameter appear to be the same.
     *  This is not an absolute test */
    bool equals(ConfigParam* other) const
    {
        return (m_param_type == other->m_param_type) and
               (m_param_name == other->m_param_name) and
               (m_special_type == other->m_special_type);
    }

    int getIntValue();
    bool getBoolValue();
    float getFloatValue();
    std::string getStringValue();

    void setIntValue(const int newValue);
    void setBoolValue(const int newValue);
    void setFloatValue(const float newValue);
    void setStringValue(const std::string& newValue);
};

class ConfigSection
{
public:
    std::string              m_section_name;
    std::vector<ConfigParam> m_parameters;
    m64p_handle m_handle;

    ConfigSection(std::string name, m64p_handle sectionHandle) : m_handle(sectionHandle)
    {
        m_section_name = name;
    }

    bool hasChildNamed(const char* name) const;
    ConfigParam* getParamWithName(const char* name);

    /**
     * Add a new parameter to this section, if not already present
     * @param name     Name of the new parameter to add
     * @param help  Help string associated with this new parameter (may be NULL)
     * @param type  Type of the new parameter
     * @param value Default value for this parameter
     * @param specialtype Additionnal information about this parameter
     */
    void addNewParam(const char* name, const char* help, wxVariant value, m64p_type type,
                      SpecialParamType specialtype = NOTHING_SPECIAL);

    static bool compare(const ConfigSection& a, const ConfigSection& b)
    {
        return (a.m_section_name < b.m_section_name);
    }
};

class IEmuStateListener
{
public:
    virtual void onStateChanged(m64p_emu_state newState) = 0;
    virtual void onSaveSlotChanged(int saveSlot) = 0;
};

class Mupen64PlusPlus
{
    void loadPlugins();

    std::string m_defaultPluginPath;
    std::string m_defaultVideoPlugin;
    std::string m_defaultAudioPlugin;
    std::string m_defaultInputPlugin;
    std::string m_defaultRspPlugin;
 
    bool m_is_a_rom_open;
 
    /** Size of the currently open ROM, in bytes */
    int m_curr_rom_size;
 
    static void StateCallback(void *Context, m64p_core_param param_type, int new_value);
    void onStateChanged(m64p_core_param param_type, int new_value);
 
    IEmuStateListener* m_listener;
 
public:

    struct RomInfo
    {
        wxString name;
        wxString goodname;
        wxString country;
        int CRC1;
        int CRC2;
        int size;
        unsigned int manufacturer;
        
        RomInfo()
        {
            CRC1 = 0;
            CRC2 = 0;
            size = -1;
            manufacturer = 0;
        }
    };

    Mupen64PlusPlus(const char *CoreLibFilepath, const char* defaultPluginPath,
                    const char* defaultVideoPlugin, const char* defaultAudioPlugin,
                    const char* defaultInputPlugin, const char* defaultRspPlugin,
                    const char* datapath);

    ~Mupen64PlusPlus();

    /** Retrieve configuration through the config API */
    std::vector<ConfigSection> getConfigContents();

    /** Write the config file with any modified values any parameter may have had since reading it */
    m64p_error saveConfig();
    
    /**
     * @pre The emulator cannot be currently running. A ROM image must not be currently opened
     * @param attachPlugins whether to automatically attach plugings upon reading the ROM.
     *                      Should be true if you open the ROM to play it, false if you open the
     *                      ROM to get information from its headers, for instance.
     */
    void loadRom(wxString filename, bool attachPlugins=true, wxProgressDialog* dlg=NULL);

    /**
     * @pre The emulator cannot be currently running. A ROM image must have been previously opened.
     */
    void closeRom(bool detachPlugins=true);
    
    bool isARomOpen() const { return m_is_a_rom_open; }
    
    /**
     * @pre The emulator cannot be currently running. A ROM image must have been previously opened.
     * @note This function call will not return until the game has been stopped. 
     */
    void runEmulation();
    
    /**
     * Get info about the currently loaded ROM
     */
    RomInfo getRomInfo();
    
    /**
     * Get info about any ROM from disk, without loading it (this variant does include as much
     * informaion as the getRomInfo() function that returns information about the currently loaded
     * ROM; for instance, the good name will not be available in the struct returned by this mehtod)
     */
    RomInfo getRomInfo(wxString path);
    
    /**
     * This will stop the emulator, if it is currently running.
     * @note This command will execute asynchronously. 
     */
    void stopEmulation();
    
    /**
     * This command will pause the emulator if it is running.
     * @return This function will return a non-successful error code if the emulator is in the stopped state.
     * @note This command may execute asynchronously. 
     */
    void pauseEmulation();
    
    /**
     * This command will resume execution of the emulator if it is paused. 
     * @return This function will return a non-successful error code if the emulator is in the stopped state.
     * @note This command may execute asynchronously. 
     */
    void resumeEmulation();
    
    /**
     * @return whether the emulator is running, paused or stopped
     */
    m64p_emu_state getEmulationState();
    
    int getSaveSlot();
    void setSaveSlot(int slot);
    
    /**
     * Save the game.
     * @param pj64Format If true, a Project64 compatible save is produced. Otherwise, a regular mupen save is generated.
     * @param path       If set, the savegame is written at the given path
     */
    void saveGame(bool pj64Format = false, char* path = NULL);
    
    /**
     * This will cause the core to save a screenshot at the next possible opportunity. 
     * The emulator must be currently running or paused. This command will execute asynchronously.
     */
    void takeScreenshot();
    
    /**
     * Set a new listener. Does not take ownership (will not delete it). Only one listener may be set
     * at a time. Passing NULL clears the current listener.
     */
    void setListener(IEmuStateListener* listener) { m_listener = listener; }
    
};

#endif

