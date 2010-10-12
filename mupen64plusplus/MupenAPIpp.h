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

    /** A key binding, stored using "key(111)" format in config */
    BINDING_STRING,
    
    /** A key binding, stored using "key(111, 222)" format in config */
    BINDING_DOUBLE_STRING,
    
    /** A directory parameter (primitive type = string)*/
    DIRECTORY
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

    SpecialParamType m_special_type;

    /** will be non-empty if some pre-defined choices are available */
    std::vector<ConfigParamChoice> m_choices;

    ConfigParam(m64p_handle section, SpecialParamType type = NOTHING_SPECIAL)
    {
        m_enabled = true;
        m_parent_section = section;
        m_special_type = type;
        m_magic_number = 0xC001C001;
    }

    ~ConfigParam()
    {
        m_magic_number = 0xDEADBEEF;
    }

    bool ok() const { return (m_magic_number == 0xC001C001); }

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

    void addNewParam(const char* name, wxVariant value, m64p_type type, SpecialParamType type = NOTHING_SPECIAL);

    static bool compare(const ConfigSection& a, const ConfigSection& b)
    {
        return (a.m_section_name < b.m_section_name);
    }
};

class Mupen64PlusPlus
{
    void loadPlugins();

public:

    Mupen64PlusPlus(const char *CoreLibFilepath, const char* defaultPluginPath,
                    const char* defaultVideoPlugin, const char* defaultAudioPlugin,
                    const char* defaultInputPlugin, const char* defaultRspPlugin);


    ~Mupen64PlusPlus();

    std::vector<ConfigSection> getConfigContents();

    m64p_error saveConfig();
};

#endif

