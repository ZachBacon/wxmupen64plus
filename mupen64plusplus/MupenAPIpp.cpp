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

#include "mupen64plusplus/MupenAPIpp.h"
#include "mupen64plusplus/MupenAPI.h"
#include "mupen64plusplus/plugin.h"
#include <stdexcept>
#include <string>
#include <wx/stream.h>
#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/intl.h>
#include <SDL.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

Mupen64PlusPlus::Mupen64PlusPlus(const char *CoreLibFilepath, const char* defaultPluginPath,
                                 const char* defaultVideoPlugin, const char* defaultAudioPlugin,
                                 const char* defaultInputPlugin, const char* defaultRspPlugin)
{
    m_defaultPluginPath = defaultPluginPath;
    m_defaultVideoPlugin = defaultVideoPlugin;
    m_defaultAudioPlugin = defaultAudioPlugin;
    m_defaultInputPlugin = defaultInputPlugin;
    m_defaultRspPlugin = defaultRspPlugin;
    
    m64p_error result = AttachCoreLib(CoreLibFilepath);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::Mupen64PlusPlus] AttachCoreLib failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    result = InitCore();
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::Mupen64PlusPlus] InitCore failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    result = OpenConfigurationHandles(defaultPluginPath, defaultVideoPlugin, defaultAudioPlugin,
                                     defaultInputPlugin, defaultRspPlugin);
    if (result != M64ERR_SUCCESS)
    {
        (*CoreShutdown)();
        DetachCoreLib();
        std::string errmsg = "[Mupen64PlusPlus::Mupen64PlusPlus] OpenConfigurationHandles failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    loadPlugins();
}

Mupen64PlusPlus::~Mupen64PlusPlus()
{
    (*PluginUnload)();
    (*CoreShutdown)();
    DetachCoreLib();
}

m64p_error Mupen64PlusPlus::saveConfig()
{
    return ::saveConfig();;
}

void Mupen64PlusPlus::loadPlugins()
{
    char pluginsPath[1024];
    char videoPlugin[128];
    char audioPlugin[128];
    char inputPlugin[128];
    char rspPlugin[128];

    m64p_error result = GetConfigPlugins(pluginsPath, 512,
                                         videoPlugin, 128,
                                         audioPlugin, 128,
                                         inputPlugin, 128,
                                         rspPlugin,   128);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::loadPlugins] GetConfigPlugins failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    g_PluginDir   = pluginsPath;
    g_GfxPlugin   = videoPlugin;
    g_AudioPlugin = audioPlugin;
    g_InputPlugin = inputPlugin;
    g_RspPlugin   = rspPlugin;
    
    result = PluginSearchLoad(getConfigUI());
    if (result != M64ERR_SUCCESS)
    {
        // if loading plugins failed, try if default path works any better (this is especially needed on OS X
        // where the application can be moved around, and thus break the path stored in the config)
        g_PluginDir = m_defaultPluginPath.c_str();
        result = PluginSearchLoad(getConfigUI());
        if (result == M64ERR_SUCCESS)
        {
            // store updated path in config
            m64p_handle uisection = getConfigUI();
            setStringConfigParam(uisection, "PluginDir", g_PluginDir);
        }
        else
        {
            // if loading plugins failed, offer to fall back to defaults
            //wxMessageBox (wxString::Format(_("Failed to load plugins, reason : <%s>\n\nWould you like to revert to def"), getErrorMessage(result)), _("Plugin(s) not found"), int style=wxOK
            std::string errmsg = "[Mupen64PlusPlus::loadPlugins] PluginSearchLoad failed with error : ";
            errmsg = errmsg + getErrorMessage(result);
            throw std::runtime_error(errmsg);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------

// FIXME: avoid the use of globals if possible
std::vector<ConfigSection> g_config_sections;

void ParameterListCallback(void* sectionHandle, const char* ParamName, m64p_type ParamType)
{
    m64p_handle* section = (m64p_handle*)sectionHandle;

    ConfigParam param(*section);
    param.m_param_type = ParamType;
    param.m_param_name = ParamName;

    const char* help = getParameterHelp(section, ParamName);
    if (help != NULL) param.m_help_string = help;

    g_config_sections[g_config_sections.size() - 1].m_parameters.push_back(param);
}

// -----------------------------------------------------------------------------------------------------------

void SectionListCallback(void* context, const char* SectionName)
{
    m64p_handle handle = getSectionHandle(SectionName);
    g_config_sections.push_back(ConfigSection(SectionName, handle));

    m64p_error result = ReadConfigSectionParameters(SectionName, &ParameterListCallback);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::getConfigContents] ReadConfigSectionParameters failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

std::vector<ConfigSection> Mupen64PlusPlus::getConfigContents()
{
    g_config_sections.clear();

    m64p_error result = ReadConfigSections(&SectionListCallback);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::getConfigContents] ReadConfigSectionParameters failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    return g_config_sections;
}

// -----------------------------------------------------------------------------------------------------------

void Mupen64PlusPlus::loadRom(wxString filename, bool attachPlugins)
{
    // SDL_Quit();
    
    wxFileInputStream input(filename);
    if (!input.IsOk())
    {
        throw std::runtime_error(("[Mupen64PlusPlus::loadRom] failed to open file '" + filename + "'").ToStdString());
    }
    wxMemoryOutputStream memoryImage;
    input.Read(memoryImage);
    
    wxStreamBuffer* buffer = memoryImage.GetOutputStreamBuffer();
    
    m64p_error result = ::openRom(buffer->GetBufferSize(), buffer->GetBufferStart());
    
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Loading ROM failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
    
    if (attachPlugins)
    {
        result = ::attachPlugins();
        if (result != M64ERR_SUCCESS)
        {
            std::string errmsg = "Attaching plugins when opening ROM failed with error : ";
            errmsg = errmsg + getErrorMessage(result);
            throw std::runtime_error(errmsg);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------

void Mupen64PlusPlus::closeRom(bool detachPlugins)
{
    if (detachPlugins)
    {
        m64p_error result = ::detachPlugins();
        if (result != M64ERR_SUCCESS)
        {
            std::string errmsg = "Detaching plugins when closing ROM failed with error : ";
            errmsg = errmsg + getErrorMessage(result);
            throw std::runtime_error(errmsg);
        }
    }
    
    m64p_error result = ::closeRom();
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Closing ROM failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

Mupen64PlusPlus::RomInfo Mupen64PlusPlus::getRomInfo()
{
    m64p_rom_header header;
    m64p_rom_settings settings;
    
    m64p_error result = getRomHeader(&header);
    
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Reading ROM header failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
    
    result = getRomSettings(&settings);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Reading ROM settings failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
    
    RomInfo out;
    
    unsigned short countrycode = header.Country_code;
    unsigned int manufacturer = header.Manufacturer_ID;
    out.name = header.Name;
    out.goodname = settings.goodname;
    out.CRC1 = header.CRC1;
    out.CRC2 = header.CRC2;
    
    switch(countrycode)
    {
    case 0x41:
        out.country = _("Japan/USA");
        break;

    case 0x44:
        out.country = _("Germany");
        break;

    case 0x45:
        out.country = _("USA");
        break;

    case 0x46:
        out.country = _("France");
        break;

    case 'I':
        out.country = _("Italy");
        break;

    case 0x4A:
        out.country = _("Japan");
        break;

    case 'S':
        out.country = _("Spain");
        break;

    case 0x55: case 0x59:
        out.country = _("Australia");
        break;

    case 0x50: case 0x58: case 0x20:
    case 0x21: case 0x38: case 0x70:
        out.country = _("Europe");
        break;

    default:
        out.country = _("Other");
        break;
    }
    return out;
}

// -----------------------------------------------------------------------------------------------------------

void Mupen64PlusPlus::runEmulation()
{
    m64p_error result = ::runEmulation();
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Running emulation failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

void Mupen64PlusPlus::stopEmulation()
{
    m64p_error result = ::stopEmulation();
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Stopping emulation failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

void Mupen64PlusPlus::pauseEmulation()
{
    m64p_error result = ::pauseEmulation();
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Pausing emulation failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

void Mupen64PlusPlus::resumeEmulation()
{
    m64p_error result = ::resumeEmulation();
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "Resuming emulation failed with error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int ConfigParam::getIntValue()
{
    assert(m_magic_number == 0xC001C001);

    if (m_param_type != M64TYPE_INT)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getIntValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " is not an int";
        throw std::runtime_error(errmsg);
    }

    int value = -1;
    m64p_error result = getIntConfigParam(m_parent_section, m_param_name.c_str(), &value);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getIntValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be read, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    return value;
}

// -----------------------------------------------------------------------------------------------------------

bool ConfigParam::getBoolValue()
{
    assert(m_magic_number == 0xC001C001);

    if (m_param_type != M64TYPE_BOOL)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getBoolValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " is not a boolean";
        throw std::runtime_error(errmsg);
    }

    int value = 0;
    m64p_error result = getBoolConfigParam(m_parent_section, m_param_name.c_str(), &value);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getBoolValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be read, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    return value != 0;
}

// -----------------------------------------------------------------------------------------------------------

float ConfigParam::getFloatValue()
{
    assert(m_magic_number == 0xC001C001);

    if (m_param_type != M64TYPE_FLOAT)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getFloatValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " is not a float";
        throw std::runtime_error(errmsg);
    }

    float value = -1;
    m64p_error result = getFloatConfigParam(m_parent_section, m_param_name.c_str(), &value);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getFloatValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be read, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    return value;
}

// -----------------------------------------------------------------------------------------------------------

std::string ConfigParam::getStringValue()
{
    assert(m_magic_number == 0xC001C001);

    if (m_param_type != M64TYPE_STRING)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getStringValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " is not a string";
        throw std::runtime_error(errmsg);
    }

    const int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    
    m64p_error result = getStringConfigParam(m_parent_section, m_param_name.c_str(),
                                             buffer, BUFFER_SIZE);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::getStringValue()] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be read, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }

    return std::string(buffer);
}

// -----------------------------------------------------------------------------------------------------------

void ConfigParam::setIntValue(const int newValue)
{
    assert(m_magic_number == 0xC001C001);

    m64p_error result = setIntConfigParam(m_parent_section, m_param_name.c_str(), newValue);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::setIntValue(";
        errmsg += newValue;
        errmsg += ")] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be written to, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

void ConfigParam::setBoolValue(const int newValue)
{
    assert(m_magic_number == 0xC001C001);

    m64p_error result = setBoolConfigParam(m_parent_section, m_param_name.c_str(), newValue);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::setBoolValue(";
        errmsg += newValue;
        errmsg += ")] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be written to, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

void ConfigParam::setFloatValue(const float newValue)
{
    assert(m_magic_number == 0xC001C001);

    m64p_error result = setFloatConfigParam(m_parent_section, m_param_name.c_str(), newValue);
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::setFloatValue(";
        errmsg += newValue;
        errmsg += ")] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be written to, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------

void ConfigParam::setStringValue(const std::string& newValue)
{
    assert(m_magic_number == 0xC001C001);

    m64p_error result = setStringConfigParam(m_parent_section, m_param_name.c_str(), newValue.c_str());
    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigParam::setStringValue(";
        errmsg += newValue;
        errmsg += ")] parameter ";
        errmsg += m_param_name;
        errmsg += " value could not be written to, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

bool ConfigSection::hasChildNamed(const char* name) const
{
    const int count = m_parameters.size();
    for (int n=0; n<count; n++)
    {
        if (m_parameters[n].m_param_name == name) return true;
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------

ConfigParam* ConfigSection::getParamWithName(const char* name)
{
    const int count = m_parameters.size();
    for (int n=0; n<count; n++)
    {
        if (m_parameters[n].m_param_name == name) return &m_parameters[n];
    }
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------

void ConfigSection::addNewParam(const char* name, wxVariant value, m64p_type type, SpecialParamType sptype)
{
    ConfigParam newParam(m_handle, sptype);
    newParam.m_param_type = type;
    newParam.m_param_name = name;
    m_parameters.push_back(newParam);

    // TODO: allow specifying help string
    const char* help = NULL;

    m64p_error result;
    switch (type)
    {
        case M64TYPE_BOOL:
            result = createBoolConfigParam(m_handle, name, value.GetBool(), help);
            break;
    
        case M64TYPE_INT:
            result = createIntConfigParam(m_handle, name, value.GetLong(), help);
            break;

        case M64TYPE_FLOAT:
            result = createFloatConfigParam(m_handle, name, value.GetDouble(), help);
            break;

        case M64TYPE_STRING:
            result = createStringConfigParam(m_handle, name, value.GetString().mb_str(), help);
            break;

        default:
            throw std::runtime_error("Unknown type passed to ConfigSection::addNewParam");
    }

    if (result != M64ERR_SUCCESS)
    {
        std::string errmsg = "[Mupen64PlusPlus::ConfigSection::addNewParam(";
        errmsg += name;
        errmsg += ")] ";
        errmsg += " parameter could not be created, error : ";
        errmsg = errmsg + getErrorMessage(result);
        throw std::runtime_error(errmsg);
    }
}

// -----------------------------------------------------------------------------------------------------------
