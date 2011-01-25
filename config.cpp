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
 
#include "config.h"
#include "mupen64plusplus/MupenAPI.h"
#include <wx/intl.h>

extern wxString datadir;

#define CHATTY 0

void getOptions(Mupen64PlusPlus* api, ptr_vector<ConfigSection>* out)
{
    // FIXME: this entire function could be improved; at this point, the structure of the config is
    //        hardcoded. Ideally this would be loaded ffrom some config file (but even more ideally
    //        the mupen core would provide me with this information)
    
    api->getConfigContents(out);
    
    // For now give an untranslated name to this section, this will allow us to identify it;
    // we'll translate the name later (FIXME: unclean)
    ConfigSection* inputSection = new ConfigSection( "Input", getSectionHandle("Core") ); 
    
    const int configSize = out->size();
    for (int n=0; n<configSize; n++)
    {
        ConfigSection* section = out->get(n);
        
#if CHATTY
        printf("==== Section [%s] ====\n", section->m_section_name.c_str());
#endif

        for (int p=0; p<section->m_parameters.size(); p++)
        {
#if CHATTY
            const char* type = "other";
            char buffer[256];

            switch (section->m_parameters[p].m_param_type)
            {
                case M64TYPE_INT:
                    type = "int";
                    sprintf(buffer, "%i", section->m_parameters[p].getIntValue());
                    break;
                case M64TYPE_FLOAT:
                    type = "float";
                    sprintf(buffer, "%f", section->m_parameters[p].getFloatValue());
                    break;
                case M64TYPE_BOOL:
                    type = "bool";
                    sprintf(buffer, "%i", section->m_parameters[p].getBoolValue());
                    break;
                case M64TYPE_STRING:
                    type = "string";
                    sprintf(buffer, "%s", section->m_parameters[p].getStringValue().c_str());
                    break;
            }
            
            printf("    - %s %s (%s) = %s\n",
                   type,
                   section->m_parameters[p].m_param_name.c_str(),
                   section->m_parameters[p].m_help_string.c_str(),
                   buffer);
#endif

            wxString param_wxname(section->m_parameters[p]->m_param_name);
            
            // Move key mappings from wherever they are into a separate input section
            if (param_wxname.StartsWith("Kbd Mapping"))
            {
                section->m_parameters[p]->m_special_type = KEYBOARD_KEY_INT;
                assert(section->m_parameters[p]->ok());
                ConfigParam* newp = new ConfigParam(section->m_parameters[p]);
                assert(newp->ok());
                inputSection->m_parameters.push_back(newp);
                section->m_parameters[p]->m_enabled = false;
            }
            else if (param_wxname.StartsWith("Joy Mapping"))
            {
                section->m_parameters[p]->m_special_type = BINDING_DIGITAL_STRING;
                assert(section->m_parameters[p]->ok());
                ConfigParam* newp = new ConfigParam(section->m_parameters[p]);
                assert(newp->ok());
                inputSection->m_parameters.push_back(newp);
                section->m_parameters[p]->m_enabled = false;
            }
            else if (param_wxname == "PluginDir" || param_wxname == "ScreenshotPath" ||
                     param_wxname == "SaveStatePath" || param_wxname == "SharedDataPath")
            {
                section->m_parameters[p]->m_special_type = DIRECTORY;
            }
            else if (param_wxname == "R4300Emulator")
            {
                ConfigParam* param = section->m_parameters[p];
                param->m_choices.push_back( ConfigParamChoice(_("Pure Interpreter"), 0) );
                param->m_choices.push_back( ConfigParamChoice(_("Cached Interpreter"), 1) );
                param->m_choices.push_back( ConfigParamChoice(_("Dynamic Recompiler"), 2) );
            }
        } // end for each parameter
        
        if (wxString(section->m_section_name).StartsWith("Video"))
        {
            if (wxString(section->m_section_name).Contains("Rice"))
            {
                ConfigParam* frameBufferSetting = section->getParamWithName("FrameBufferSetting");
                if (frameBufferSetting != NULL)
                {
                    frameBufferSetting->m_choices.push_back( ConfigParamChoice(_("ROM Default"), 0) );
                    frameBufferSetting->m_choices.push_back( ConfigParamChoice(_("Disabled"), 1) );
                }
                
                ConfigParam* rttSetting = section->getParamWithName("RenderToTexture");
                if (rttSetting != NULL)
                {
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("None"), 0) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Ignore"), 1) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Normal"), 2) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Write Back"), 3) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Write Back and Reload"), 4) );
                }
                
                ConfigParam* screenUpSetting = section->getParamWithName("ScreenUpdateSetting");
                if (screenUpSetting != NULL)
                {
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("ROM Default"), 0) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("VI Origin Update"), 1) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("VI Origin Change"), 2) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("CI Change"), 3) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("First CI Change"), 4) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("First Primitive Drawn"), 5) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("Before Screen Clear"), 6) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("After Screen Clear"), 7) );
                }
                
                ConfigParam* fogSetting = section->getParamWithName("FogMethod");
                if (fogSetting != NULL)
                {
                    fogSetting->m_choices.push_back( ConfigParamChoice(_("Disable"), 0) );
                    fogSetting->m_choices.push_back( ConfigParamChoice(_("Enable N64 Choose"), 1) );
                    fogSetting->m_choices.push_back( ConfigParamChoice(_("Force Fog"), 2) );                    
                }
                
                ConfigParam* forceTextureFilterSetting = section->getParamWithName("ForceTextureFilter");
                if (forceTextureFilterSetting != NULL)
                {
                    forceTextureFilterSetting->m_choices.push_back( ConfigParamChoice(_("Auto (N64 Choose)"), 0) );
                    forceTextureFilterSetting->m_choices.push_back( ConfigParamChoice(_("Force no Filtering"), 1) );
                    forceTextureFilterSetting->m_choices.push_back( ConfigParamChoice(_("Force Filtering"), 2) );                    
                }
                
                ConfigParam* textureFilteringMethod = section->getParamWithName("TextureFilteringMethod");
                if (textureFilteringMethod != NULL)
                {
                    textureFilteringMethod->m_choices.push_back( ConfigParamChoice(_("No Filtering"), 0) );
                    textureFilteringMethod->m_choices.push_back( ConfigParamChoice(_("Bilinear"), 1) );
                    textureFilteringMethod->m_choices.push_back( ConfigParamChoice(_("Trilinear"), 2) );                    
                }
                
                ConfigParam* textureEnhancement = section->getParamWithName("TextureEnhancement");
                if (textureEnhancement != NULL)
                {
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("None"), 0) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("2X"), 1) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("2XSAI"), 2) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("HQ2X"), 3) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("LQ2X"), 4) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("HQ4X"), 5) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("Sharpen"), 6) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("Sharpen More"), 7) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("External"), 8) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("Mirror"), 9) );
                }
                
                ConfigParam* textureQuality = section->getParamWithName("TextureQuality");
                if (textureQuality != NULL)
                {
                    textureQuality->m_choices.push_back( ConfigParamChoice(_("Defaut"), 0) );
                    textureQuality->m_choices.push_back( ConfigParamChoice(_("32 Bits"), 1) );
                    textureQuality->m_choices.push_back( ConfigParamChoice(_("16 Bits"), 2) );
                }
                
                ConfigParam* multiSampling = section->getParamWithName("MultiSampling");
                if (multiSampling != NULL)
                {
                    multiSampling->m_choices.push_back( ConfigParamChoice(_("Off"), 0) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("2", 2) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("4", 4) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("8", 8) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("16", 16) );
                }
                
                ConfigParam* colorQuality = section->getParamWithName("ColorQuality");
                if (colorQuality != NULL)
                {
                    colorQuality->m_choices.push_back( ConfigParamChoice(_("32 Bits"), 0) );
                    colorQuality->m_choices.push_back( ConfigParamChoice(_("16 Bits"), 1) );
                }
                
                ConfigParam* openGLDepthBufferSetting = section->getParamWithName("OpenGLDepthBufferSetting");
                if (openGLDepthBufferSetting != NULL)
                {
                    openGLDepthBufferSetting->m_choices.push_back( ConfigParamChoice(_("32 Bits"), 32) );
                    openGLDepthBufferSetting->m_choices.push_back( ConfigParamChoice(_("16 Bits"), 16) );
                }
                
                ConfigParam* openGLRenderSetting = section->getParamWithName("OpenGLRenderSetting");
                if (openGLRenderSetting != NULL)
                {
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice(_("Auto"), 0) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.1", 1) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.2", 2) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.3", 3) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.4", 4) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.4 v2", 5) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL TNT2", 6) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("NVIDIA OGL", 7) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice(_("OGL Fragment Program"), 8) );
                }
            }
        }
        else if (section->m_section_name == "Audio-SDL")
        {
            ConfigParam* resample = section->getParamWithName("RESAMPLE");
            if (resample != NULL)
            {
                resample->m_choices.push_back( ConfigParamChoice(_("Unfiltered"), 1) );
                resample->m_choices.push_back( ConfigParamChoice(_("SINC Resampling"), 2) );
            }
            
            ConfigParam* volumeCtrType = section->getParamWithName("VOLUME_CONTROL_TYPE");
            if (volumeCtrType != NULL)
            {
                volumeCtrType->m_choices.push_back( ConfigParamChoice(_("SDL (mupen output only)"), 1) );
                volumeCtrType->m_choices.push_back( ConfigParamChoice(_("OSS mixer (master PC volume)"), 2) );
            }
        }
        else if (wxString(section->m_section_name.c_str()).StartsWith("Input-SDL-Control"))
        {
            // Add any missing parameter
#define CREATE_PARAM_IF_MISSING( name, val, type, disp, help )                  \
            if (!section->hasChildNamed(name))                                   \
                section->addNewParam(name, help, wxVariant( val ), type, disp);  \
            else                                                                \
                section->getParamWithName(name)->m_special_type = disp
            
            try
            {
                CREATE_PARAM_IF_MISSING("plugged",           false,         M64TYPE_BOOL,   NOTHING_SPECIAL, "Specifies whether this input device is currently enabled");
                CREATE_PARAM_IF_MISSING("plugin",            1,             M64TYPE_INT,    NOTHING_SPECIAL, "Expansion pack type, if any");
                CREATE_PARAM_IF_MISSING("mouse",             false,         M64TYPE_BOOL,   NOTHING_SPECIAL, "Whether mouse use is enabled");
                CREATE_PARAM_IF_MISSING("device",            -2,            M64TYPE_INT,    NOTHING_SPECIAL, "Specifies which input device to use");
                
                // TODO: provide nicer way to edit deadzone and peak
                CREATE_PARAM_IF_MISSING("AnalogDeadzone",    "4096,4096",   M64TYPE_STRING, NOTHING_SPECIAL, "For analog controls, specifies the dead zone");
                CREATE_PARAM_IF_MISSING("AnalogPeak",        "32768,32768", M64TYPE_STRING, NOTHING_SPECIAL, "For analog controls, specifies the peak value");
                CREATE_PARAM_IF_MISSING("DPad R",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Right button on the digital pad");
                CREATE_PARAM_IF_MISSING("DPad L",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Left button on the digital pad");
                CREATE_PARAM_IF_MISSING("DPad D",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Down button on the digital pad");
                CREATE_PARAM_IF_MISSING("DPad U",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Up button on the digital pad");
                CREATE_PARAM_IF_MISSING("Start",             "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("Z Trig",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("B Button",          "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("A Button",          "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("C Button R",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Right button");
                CREATE_PARAM_IF_MISSING("C Button L",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Left button");
                CREATE_PARAM_IF_MISSING("C Button D",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Down button");
                CREATE_PARAM_IF_MISSING("C Button U",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Up button");
                CREATE_PARAM_IF_MISSING("R Trig",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("L Trig",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("Mempak switch",     "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("Rumblepak switch",  "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("X Axis",            "", M64TYPE_STRING, BINDING_ANALOG_COUPLE_STRING, "Horizontal analog axis");
                CREATE_PARAM_IF_MISSING("Y Axis",            "", M64TYPE_STRING, BINDING_ANALOG_COUPLE_STRING, "Vertical analog axis");
            }
            catch (std::exception& e)
            {
                fprintf(stderr, "Error caught while trying to set up %s : %s\n", (const char*)section->m_section_name.c_str(), e.what());
            }

            ConfigParam* x_axis = section->getParamWithName("X Axis");
            x_axis->m_icon_1 = datadir + "left.png";
            x_axis->m_icon_2 = datadir + "right.png";
            ConfigParam* y_axis = section->getParamWithName("Y Axis");
            y_axis->m_icon_1 = datadir + "up.png";
            y_axis->m_icon_2 = datadir + "down.png";
            
#undef CREATE_PARAM_IF_MISSING

            ConfigParam* pluginParam = section->getParamWithName("plugin");
            if (pluginParam != NULL)
            {
                pluginParam->m_choices.push_back( ConfigParamChoice(_("None"), 1) );
                pluginParam->m_choices.push_back( ConfigParamChoice(_("Mem Pack"), 2) );
                pluginParam->m_choices.push_back( ConfigParamChoice(_("Rumble Pack"), 5) );
            }

            ConfigParam* deviceParam = section->getParamWithName("device");
            if (deviceParam != NULL)
            {
                deviceParam->m_choices.push_back( ConfigParamChoice(_("Keyboard/Mouse"), -2) );
                deviceParam->m_choices.push_back( ConfigParamChoice(_("Auto Config"), -1) );
                
                for (int n=0; n<9; n++)
                {
                    deviceParam->m_choices.push_back(
                            ConfigParamChoice(wxString::Format(_("Joystick %i"), n), n)
                        );
                }
            }
        }
        
    } // end for each section
    
    out->push_back(inputSection);
}

