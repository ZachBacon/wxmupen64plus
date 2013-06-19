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

#ifdef __WXMSW__
// Because of Windows.h issues if include order is not correct
#include <wx/wx.h>
#endif

#include "config.h"
#include "mupen64plusplus/MupenAPI.h"
#include "sdlhelper.h"
#include "main.h"
#include <wx/intl.h>

extern wxString datadir;

#define CHATTY 0

bool parameters_sort_fn(ConfigParam* a, ConfigParam* b)
{
    if (a == NULL && b == NULL) return 0;
    if (a == NULL && b != NULL) return -1;
    if (a != NULL && b == NULL) return 1;
    
    return a->getName() < b->getName();
}

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

            wxString param_wxname(section->m_parameters[p]->getName());

            // Move key mappings from wherever they are into a separate input section
            if (param_wxname.StartsWith("Kbd Mapping"))
            {
                section->m_parameters[p]->m_special_type = KEYBOARD_KEY_INT;
                assert(section->m_parameters[p]->ok());
                ConfigParam* newp = new ConfigParam(section->m_parameters[p]);
                assert(newp->ok());
                inputSection->m_parameters.push_back(newp);
                section->m_parameters[p]->setEnabled( false );
            }
            else if (param_wxname.StartsWith("Joy Mapping"))
            {
                section->m_parameters[p]->m_special_type = BINDING_DIGITAL_STRING;
                assert(section->m_parameters[p]->ok());
                ConfigParam* newp = new ConfigParam(section->m_parameters[p]);
                assert(newp->ok());
                inputSection->m_parameters.push_back(newp);
                section->m_parameters[p]->setEnabled( false );
            }
            else if (param_wxname == "PluginDir" || param_wxname == "ScreenshotPath" ||
                     param_wxname == "SaveStatePath" || param_wxname == "SharedDataPath")
            {
                section->m_parameters[p]->m_special_type = DIRECTORY;
            }
            else if (param_wxname == "VideoExtension")
            {
                ConfigParam* param = section->m_parameters[p];
                param->m_choices.push_back( ConfigParamChoice(_("Internal"), std::string("internal") ) );
                param->m_choices.push_back( ConfigParamChoice(_("External"), std::string("external") ) );
                param->setCommentString(_("* Changes to this parameter will be only effective after restarting Mupen64Plus"));
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
            ConfigParam* screenH = section->getParamWithName("ScreenHeight");
            if (screenH != NULL)
            {
                screenH->setCommentString(_("* Size parameters are only used if external video is used. If you use one-window-mupen,\nsimply resize the window. This setting can be changed in the 'Plugins' section."));
            }
            
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

                ConfigParam* mipmapSetting = section->getParamWithName("Mipmapping");
                if (mipmapSetting != NULL)
                {
                    mipmapSetting->m_choices.push_back( ConfigParamChoice(_("Disable"), 0) );
                    mipmapSetting->m_choices.push_back( ConfigParamChoice(_("Nearest"), 1) );
                    mipmapSetting->m_choices.push_back( ConfigParamChoice(_("Bilinear"), 2) );
                    mipmapSetting->m_choices.push_back( ConfigParamChoice(_("Trilinear"), 3) );
                }
                
                ConfigParam* anisotropicSetting = section->getParamWithName("AnisotropicFiltering");
                if (anisotropicSetting != NULL)
                {
                    anisotropicSetting->m_choices.push_back( ConfigParamChoice(_("Disable"), 0) );
                    anisotropicSetting->m_choices.push_back( ConfigParamChoice(_("2x"), 2) );
                    anisotropicSetting->m_choices.push_back( ConfigParamChoice(_("4x"), 4) );
                    anisotropicSetting->m_choices.push_back( ConfigParamChoice(_("8x"), 8) );
                    anisotropicSetting->m_choices.push_back( ConfigParamChoice(_("16x"), 16) );
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
            else if (wxString(section->m_section_name).Contains("Glide64"))
            {
                section->m_parameters.sort(&parameters_sort_fn);
                
                ConfigParam* filteringSetting = section->getParamWithName("filtering");
                if (filteringSetting != NULL)
                {
                    filteringSetting->m_choices.push_back( ConfigParamChoice(_("None"), 0) );
                    filteringSetting->m_choices.push_back( ConfigParamChoice(_("Force Bilinear"), 1) );
                    filteringSetting->m_choices.push_back( ConfigParamChoice(_("Force Point-sampled"), 2) );
                }
                
                ConfigParam* wfmode = section->getParamWithName("wfmode");
                if (wfmode != NULL)
                {
                    wfmode->setHelpString(_("Wireframe mode"));
                    wfmode->m_choices.push_back( ConfigParamChoice(_("Normal Colors"), 0) );
                    wfmode->m_choices.push_back( ConfigParamChoice(_("Vertex Colors"), 1) );
                    wfmode->m_choices.push_back( ConfigParamChoice(_("Red Only"), 2) );
                }
                
                ConfigParam* swapmode = section->getParamWithName("swapmode");
                if (swapmode != NULL)
                {
                    swapmode->setHelpString(_("Buffer Swapping Method"));
                    swapmode->m_choices.push_back( ConfigParamChoice(_("Old"), 0) );
                    swapmode->m_choices.push_back( ConfigParamChoice(_("New"), 1) );
                    swapmode->m_choices.push_back( ConfigParamChoice(_("Hybrid"), 2) );
                }
                
                ConfigParam* lodmode = section->getParamWithName("lodmode");
                if (lodmode != NULL)
                {
                    lodmode->setHelpString(_("LOD Calculation"));
                    lodmode->m_choices.push_back( ConfigParamChoice(_("Off"), 0) );
                    lodmode->m_choices.push_back( ConfigParamChoice(_("Fast"), 1) );
                    lodmode->m_choices.push_back( ConfigParamChoice(_("Precise"), 2) );
                }
                
                ConfigParam* show_fps = section->getParamWithName("show_fps");
                if (show_fps != NULL)
                {
                    show_fps->setHelpString(_("Show FPS"));
                    // TODO: allow combinable bitmasks
                    show_fps->m_choices.push_back( ConfigParamChoice(_("Disabled"), 0) );
                    show_fps->m_choices.push_back( ConfigParamChoice(_("FPS Counter"), 1) );
                    show_fps->m_choices.push_back( ConfigParamChoice(_("VI/s Counter"), 2) );
                    show_fps->m_choices.push_back( ConfigParamChoice(_("% Speed"), 4) );
                    show_fps->m_choices.push_back( ConfigParamChoice(_("FPS Transparent"), 8) );
                }
                
                ConfigParam* tex_filter = section->getParamWithName("tex_filter");
                if (tex_filter != NULL)
                {
                    tex_filter->setHelpString(_("Texture Filter"));
                    tex_filter->m_choices.push_back( ConfigParamChoice(_("Disabled"), 0) );
                    tex_filter->m_choices.push_back( ConfigParamChoice(_("Blur edges"), 1) );
                    tex_filter->m_choices.push_back( ConfigParamChoice(_("Super 2xSAI"), 2) );
                    tex_filter->m_choices.push_back( ConfigParamChoice(_("Hq2x"), 3) );
                    tex_filter->m_choices.push_back( ConfigParamChoice(_("Hq4x"), 4) );
                }
                
                ConfigParam* ghq_cmpr = section->getParamWithName("ghq_cmpr");
                if (ghq_cmpr != NULL)
                {
                    ghq_cmpr->setHelpString((_("Texture compression")));
                    ghq_cmpr->m_choices.push_back( ConfigParamChoice("S3TC", 0) );
                    ghq_cmpr->m_choices.push_back( ConfigParamChoice("FXT1", 1) );
                }
                
                ConfigParam* ghq_enht_cmpr = section->getParamWithName("ghq_enht_cmpr");
                if (ghq_enht_cmpr != NULL)
                {
                    ghq_enht_cmpr->setHelpString(_("Compress texture cache (S3TC/FXT1)"));
                }
                
                ConfigParam* ghq_enht_tile = section->getParamWithName("ghq_enht_tile");
                if (ghq_enht_tile != NULL)
                {
                    ghq_enht_tile->setHelpString(_("Tile textures"));
                }
                
                ConfigParam* ghq_enht_f16bpp = section->getParamWithName("ghq_enht_f16bpp");
                if (ghq_enht_f16bpp != NULL)
                {
                    ghq_enht_f16bpp->setHelpString(_("Force 16bpp"));
                }
                
                ConfigParam* ghq_hirs = section->getParamWithName("ghq_hirs");
                if (ghq_hirs != NULL)
                {
                    ghq_hirs->setHelpString(_("High-res texture packs"));
                    ghq_hirs->m_choices.push_back( ConfigParamChoice("None", 0) );
                    ghq_hirs->m_choices.push_back( ConfigParamChoice("Rice format", 1) );
                }
                
                ConfigParam* ghq_enht_gz = section->getParamWithName("ghq_enht_gz");
                if (ghq_enht_gz != NULL)
                {
                    ghq_enht_gz->setHelpString(_("Compress texture cache (GZ)"));
                }
                
                ConfigParam* ghq_hirs_altcrc = section->getParamWithName("ghq_hirs_altcrc");
                if (ghq_hirs_altcrc != NULL)
                {
                    ghq_hirs_altcrc->setHelpString(_("[High-res tex] Alternative CRC calculation"));
                }
                
                ConfigParam* ghq_hirs_let_texartists_fly = section->getParamWithName("ghq_hirs_let_texartists_fly");
                if (ghq_hirs_let_texartists_fly != NULL)
                {
                    ghq_hirs_let_texartists_fly->setHelpString(_("[High-res tex] Use full alpha channel"));
                }
                
                ConfigParam* ghq_hirs_f16bpp = section->getParamWithName("ghq_hirs_f16bpp");
                if (ghq_hirs_f16bpp != NULL)
                {
                    ghq_hirs_f16bpp->setHelpString(_("[High-res tex] Force 16bpp"));
                }
                
                ConfigParam* ghq_hirs_tile = section->getParamWithName("ghq_hirs_tile");
                if (ghq_hirs_tile != NULL)
                {
                    ghq_hirs_tile->setHelpString(_("[High-res tex] Tile textures"));
                }
                
                ConfigParam* ghq_hirs_gz = section->getParamWithName("ghq_hirs_gz");
                if (ghq_hirs_gz != NULL)
                {
                    ghq_hirs_gz->setHelpString(_("[High-res tex] Compress texture cache (GZ)"));
                }
                
                ConfigParam* ghq_hirs_cmpr = section->getParamWithName("ghq_hirs_cmpr");
                if (ghq_hirs_cmpr != NULL)
                {
                    ghq_hirs_cmpr->setHelpString(_("[High-res tex] Compress texture cache (S3TC/FXT1)"));
                }
                
                ConfigParam* ghq_hirs_dump = section->getParamWithName("ghq_hirs_dump");
                if (ghq_hirs_dump != NULL)
                {
                    ghq_hirs_dump->setHelpString(_("[High-res tex] Dump textures"));
                }
            }
        }
        else if (section->m_section_name == "Audio-SDL")
        {
            ConfigParam* resample = section->getParamWithName("RESAMPLE");
            if (resample != NULL)
            {
                resample->m_choices.push_back( ConfigParamChoice(_("Unfiltered"), std::string("trivial") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-0"), std::string("speex-fixed-0") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-1"), std::string("speex-fixed-1") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-2"), std::string("speex-fixed-2") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-3"), std::string("speex-fixed-3") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-4"), std::string("speex-fixed-4") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-5"), std::string("speex-fixed-5") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-6"), std::string("speex-fixed-6") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-7"), std::string("speex-fixed-7") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-8"), std::string("speex-fixed-8") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-9"), std::string("speex-fixed-9") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("speex-fixed-10"), std::string("speex-fixed-10") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("src-linear"), std::string("src-linear") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("src-zero-order-hold"), std::string("src-zero-order-hold") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("src-sinc-fastest"), std::string("src-sinc-fastest") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("src-sinc-medium-quality"), std::string("src-sinc-medium-quality") ) );
                resample->m_choices.push_back( ConfigParamChoice(_("src-sinc-best-quality"), std::string("src-sinc-best-quality") ) );
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
                CREATE_PARAM_IF_MISSING("mode",              2,             M64TYPE_INT,    INPUT_MODE_PICKER, "Input configuration mode");
                CREATE_PARAM_IF_MISSING("plugged",           false,         M64TYPE_BOOL,   OTHER_INPUT_FIELD, "Specifies whether this input device is currently enabled");
                CREATE_PARAM_IF_MISSING("plugin",            1,             M64TYPE_INT,    OTHER_INPUT_FIELD, "Expansion pack type, if any");
                CREATE_PARAM_IF_MISSING("mouse",             false,         M64TYPE_BOOL,   OTHER_INPUT_FIELD, "Whether mouse use is enabled");
                CREATE_PARAM_IF_MISSING("device",            -1,            M64TYPE_INT,    OTHER_INPUT_FIELD, "Specifies which input device to use");
                CREATE_PARAM_IF_MISSING("name",              "Keyboard",    M64TYPE_STRING, INPUT_DEVICE_NAME, "Name of the input device");

                // TODO: provide nicer way to edit deadzone and peak
                CREATE_PARAM_IF_MISSING("AnalogDeadzone",    "4096,4096",   M64TYPE_STRING, OTHER_INPUT_FIELD, "For analog controls, specifies the dead zone");
                CREATE_PARAM_IF_MISSING("AnalogPeak",        "32768,32768", M64TYPE_STRING, OTHER_INPUT_FIELD, "For analog controls, specifies the peak value");
                CREATE_PARAM_IF_MISSING("MouseSensitivity",  "2.00,2.00",   M64TYPE_STRING, OTHER_INPUT_FIELD, "For mouse input");
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
                mplog_error("Config", "Error caught while trying to set up %s : %s\n", (const char*)section->m_section_name.c_str(), e.what());
            }

            ConfigParam* x_axis = section->getParamWithName("X Axis");
            x_axis->m_icon_1 = datadir + "left.png";
            x_axis->m_icon_2 = datadir + "right.png";
            ConfigParam* y_axis = section->getParamWithName("Y Axis");
            y_axis->m_icon_1 = datadir + "up.png";
            y_axis->m_icon_2 = datadir + "down.png";

#undef CREATE_PARAM_IF_MISSING

            ConfigParam* modeParam = section->getParamWithName("mode");
            if (modeParam != NULL)
            {
                modeParam->m_choices.push_back( ConfigParamChoice(_("Manual"), 0) );
                modeParam->m_choices.push_back( ConfigParamChoice(_("Automatic with device selection"), 1) );
                modeParam->m_choices.push_back( ConfigParamChoice(_("Fully automatic"), 2) );
            }
            
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
                deviceParam->m_choices.push_back( ConfigParamChoice(_("No joystick"), -1) );
                
                const std::map<int, std::string>& gamepads = getGamepadList();

                for (int n=0; n<9; n++)
                {
					wxString name = _("Not plugged");
					if (gamepads.find(n) != gamepads.end())
					{
						name = gamepads.find(n)->second.c_str();
					}

                    deviceParam->m_choices.push_back(
                            ConfigParamChoice(wxString::Format(_("Joystick %i"), n) + " (" + name + ")", n)
                        );
                }
            }
        }

    } // end for each section

    out->push_back(inputSection);
}

