#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006-2010 (ita); Marianne Gagnon, 2010


# example :
# export LDFLAGS="-arch i386"
# export CXXFLAGS="-arch i386"
# export CFLAGS="-arch i386"
# waf configure --mupenapi=/Developer/hg/mupen64plus/mupen64plus-core/src/api --wxconfig=/usr/local/bin/wx-config-2.9
# waf

# the following two variables are used by the target "waf dist"
VERSION='0.4'
APPNAME='wxMupen64Plus'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

# --------------------------------------------------------------------------------------------
#                                           OPTIONS
# --------------------------------------------------------------------------------------------

def options(opt):
    import os
    
    opt.add_option('--targetos',  action='store', help='Which OS to target (auto, windows, osx, linux, freebsd)', default='auto', dest="targetos")
    opt.add_option('--mupenapi',  action='store', help='Where to find Mupen64Plus API headers (optional)', default='/usr/include')
    opt.add_option('--wxconfig',  action='store', help='Which wx-config utility to use (optional)', default='wx-config', dest="wxconfig")
    opt.add_option('--sdlconfig', action='store', help='Which sdl-config utility to use (optional)', default='sdl-config', dest="sdlconfig")
    opt.add_option('--debug',     action='store', help='Whether to make a debug build (may be \'true\' or \'false\')', default='false', dest="debugmode")
    opt.add_option('--wxconfig_args', action='store', help='Additional arguments passed to wx-config', default='',  dest='wxconfig_args')
    opt.add_option('--datadir', action='store', help='(Optional) the directory where to look for data files', default='',  dest='datadir')
    opt.add_option('--libdir', action='store', help='(Optional) the directory where to look for the core library', default='',  dest='libdir')
    opt.add_option('--pluginsdir', action='store', help='(Optional) the directory where to look for plugin files', default='',  dest='pluginsdir')
    opt.add_option('--bindir', action='store', help='(Optional) the directory where to install wxmupen64plus binary', default='',  dest='bindir')
    opt.add_option('--debugger', action='store', help='Enable or disable the debugger (true or false). Requires GCC 4.6', default='true',  dest='debugger')
    opt.add_option('--version_check', action='store', help='Enable or disable check of the mupen64plus-core version (true or false).', default='true',  dest='version_check')
    
    opt.add_option('--wxhome', action='store', help='Where your wxWidgets build is installed (windows only)', default=None,  dest='wxhome')
    opt.add_option('--wxinclude', action='store', help='Where your wxWidgets header files are installed (windows only)', default=None,  dest='wxinclude')

    opt.load('compiler_cxx')
    opt.load('compiler_c')

# --------------------------------------------------------------------------------------------
#                                   WINDOWS RC FILES BUILDER
# --------------------------------------------------------------------------------------------

#from waflib.Context import Context
#class one(Context):
#        cmd = 'windres'
#    command = ["windres", "--include-dir="+wxHomePath+"\include", "--input", "win32\Aria.rc", "--output", "msvcr.o"]
    
# --------------------------------------------------------------------------------------------
#                                          CONFIGURE
# --------------------------------------------------------------------------------------------

def configure(ctx):
    from waflib import Options, Logs
    import subprocess
    import waflib
    import os
    
    targetos   = Options.options.targetos
    if targetos == 'auto':
        if os.name == 'nt':
            targetos = 'windows'
        elif os.uname()[0] == 'Darwin':
            targetos = 'osx'
        elif os.uname()[0] == 'FreeBSD':
            targetos = 'freebsd'
        elif os.uname()[0] == 'Linux':
            targetos = 'linux'
    
    if targetos not in ('windows', 'osx', 'linux', 'freebsd'):
        waflib.Logs.error('Invalid OS "' + targetos + '", please pass a valid --targetos=... flag')
        return
    
    api_path   = Options.options.mupenapi
    wx_config  = Options.options.wxconfig
    sdl_config = Options.options.sdlconfig
    enable_debugger = (Options.options.debugger == 'true')
    version_check = (Options.options.version_check == 'true')
    
    wxconfig_args = Options.options.wxconfig_args

    wxhome = ''
    if targetos == 'windows':
        wxhome = ''.join(Options.options.wxhome)

    wxinclude = ''
    if targetos == 'windows':
        if Options.options.wxinclude == None:
            wxinclude = wxhome + r"\include"
        else:
            wxinclude = ''.join(Options.options.wxinclude)
    
    if Options.options.debugmode != 'true' and Options.options.debugmode != 'false':
        waflib.Logs.warn("Warning, the --debug option may only be 'true' or 'false'. Defaulting to 'false'.")
    
    is_debug   = (Options.options.debugmode == 'true')
    
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')

    if targetos == 'windows':
        ctx.find_program('windres', var='WINDRES', mandatory=True)
    
    ctx.env['targetos'] = targetos
    ctx.env['api_path'] = api_path
    ctx.env['is_debug'] = is_debug
    ctx.env['wxhome'] = wxhome
    ctx.env['wxinclude'] = wxinclude
    ctx.env['datadir'] = Options.options.datadir
    ctx.env['libdir'] = Options.options.libdir
    ctx.env['bindir'] = Options.options.bindir
    ctx.env['pluginsdir'] = Options.options.pluginsdir
    ctx.env['enable_debugger'] = enable_debugger

    ctx.check_cc(header_name="stdio.h", function_name='printf', msg='Checking C compiler works', errmsg="No")
    ctx.check_cxx(header_name="cstdio", function_name='printf', msg='Checking C++ compiler works', errmsg="No")
    ctx.check_cc(header_name="m64p_frontend.h",   includes=[api_path])
    ctx.check_cc(header_name="m64p_config.h",     includes=[api_path])
    ctx.check_cc(header_name="m64p_types.h",      includes=[api_path])
    if version_check:
        ctx.check_cc(header_name="../main/version.h", includes=[api_path])

    ctx.check_cfg(path=sdl_config, args='--cflags --libs',   package='', uselib_store='SDL')
    
    if targetos == 'windows':
        if wxhome == None :
            ctx.fatal("On Windows, the --wxhome argument is mandatory")
        ctx.check_cfg(msg="Checking for wxWidgets 2.9.x", path=wx_config,  args='--version=2.9 --cxxflags --prefix=' + wxhome + ' ' + wxconfig_args + ' --libs adv,aui,core,base,gl,html', package='', uselib_store='wxWidgets')
    else:
        ctx.check_cfg(msg="Checking for wxWidgets 2.9.x", path=wx_config,  args='--version=2.9 --cxxflags --libs adv,aui,core,base,gl,html ' + wxconfig_args, package='', uselib_store='wxWidgets')
    
    if targetos != 'windows' and targetos != 'osx':
        ctx.check_cxx(lib='dl')
        ctx.check_cxx(lib='X11')
        ctx.check_cxx(lib='GL')
    
    if version_check:
        ctx.check_cc(compile_filename='test.c', execute=False, cflags=["-I"+api_path], msg="Checking mupen64plus is recent enough...", fragment=
"""#include "../main/version.h"
   #if FRONTEND_API_VERSION < 0x020001
   #error Your mupen64plus build is too old, please upgrade
   #endif
   #if CONFIG_API_VERSION < 0x020000
   #error Your mupen64plus build is too old, please upgrade
   #endif
   #if VIDEXT_API_VERSION < 0x020000
   #error Your mupen64plus build is too old, please upgrade
   #endif
   int main(int argc, char** argv) {}""")

# --------------------------------------------------------------------------------------------
#                                            BUILD
# --------------------------------------------------------------------------------------------

def build(bld):
    import os

    
    targetos = bld.env['targetos']
    api_path = bld.env['api_path']
    wxhome = bld.env['wxhome']
    enable_debugger = bld.env['enable_debugger']
    
    link_flags = []
    build_flags = []
    
    # Set up debug or release build options
    if bld.env['is_debug']:
        build_flags += ['-g','-DDEBUG=1']
    else:
        build_flags += ['-O2','-DNEBUG=1']

    if len(bld.env['datadir']) > 0:
        build_flags += ['-DWXDATADIR="' + bld.env['datadir'] + '"']
    if len(bld.env['libdir']) > 0:
        build_flags += ['-DLIBDIR="' + bld.env['libdir'] + '"']
    if len(bld.env['pluginsdir']) > 0:
        build_flags += ['-DPLUGINSDIR="' + bld.env['pluginsdir'] + '"']
    if len(bld.env['bindir']) > 0:
        bin_install_path = bld.env['bindir']
    else:
        bin_install_path = "${PREFIX}/bin"

    osal_src = []
    additional_links = []
    additional_libs = []
    
    # Windows
    if targetos == 'windows':
        cmd = bld.env.WINDRES + " --include-dir=" + bld.env['wxinclude'] + " ${SRC} --output ${TGT}"
        bld(rule=cmd, source='wxmupen64plus.rc', target='manifest.o')
        additional_links += ['manifest.o']
        
        osal_src += ['mupen64plusplus/osal_dynamiclib_win32.c', 'mupen64plusplus/osal_files_win32.c']
        
    # A few OSX-specific flags
    elif targetos == 'osx':
        osal_src += ['mupen64plusplus/osal_dynamiclib_unix.c', 'mupen64plusplus/osal_files_unix.c']
        link_flags += ['-Wl,-framework,IOKit', '-Wl,-framework,Carbon',
                       '-Wl,-framework,Cocoa', '-Wl,-framework,AudioToolbox',
                       '-Wl,-framework,System', '-Wl,-framework,OpenGL',
                       '-Wl,-framework,QuickTime', '-Wl,-framework,WebKit']
        
        # install target
        bld.install_files('wxMupen64Plus.app/Contents', 'Info.plist')
        data_dir = bld.path.find_dir('data')
        bld.install_files('wxMupen64Plus.app/Contents/Resources', data_dir.ant_glob('*'))
        bin_install_path = "wxMupen64Plus.app/Contents/MacOS"
        
    elif targetos == 'freebsd':
        if 'LOCALBASE' in os.environ:
           LOCALBASE = os.environ['LOCALBASE']
        else:
           LOCALBASE = '/usr/local/'
        
        build_flags += ['-I'+ LOCALBASE +'/include/X11']
        osal_src += ['mupen64plusplus/osal_dynamiclib_unix.c', 'mupen64plusplus/osal_files_unix.c']
        
        additional_libs = ['X11', 'GL']
    else:
        # Linux
        build_flags += ['-I/usr/include/X11']
        osal_src += ['mupen64plusplus/osal_dynamiclib_unix.c', 'mupen64plusplus/osal_files_unix.c']
        
        additional_libs = ['GL', 'X11', 'dl']
        
        # install target
        data_dir = bld.path.find_dir('data')
        if len(bld.env['datadir']) > 0:
            share_path = bld.env['datadir']
        else:
            share_path = '${PREFIX}/share/wxmupen64plus/'
        bld.install_files(share_path, data_dir.ant_glob('*'))


    # Build the program
    
    if bld.env['enable_debugger']:
        debugger_sources = ['debugger/breakpoint.cpp', 'debugger/breakpointpanel.cpp', 'debugger/colors.cpp',
                            'debugger/debugconsole.cpp', 'debugger/debuggerframe.cpp', 'debugger/debugpanel.cpp',
                            'debugger/disasmpanel.cpp', 'debugger/dv_treelist.cpp', 'debugger/memorypanel.cpp',
                            'debugger/registerpanel.cpp', 'debugger/debugconfig.cpp', 'debugger/memorysearch.cpp']
        debugger_flags = ['-std=gnu++0x', '-fpermissive', '-DENABLE_DEBUGGER']
    else:
        debugger_sources = []
        debugger_flags = []
    
    bld.program(features='c cxx cxxprogram',
                cxxflags=build_flags+debugger_flags,
                cflags=build_flags+['-Wfatal-errors'],
                linkflags=link_flags + additional_links,
                source=['main.cpp', 'gamespanel.cpp', 'parameterpanel.cpp', 'sdlkeypicker.cpp',
                        'mupen64plusplus/MupenAPIpp.cpp', 'mupen64plusplus/MupenAPI.c',
                        'sdlhelper.cpp', 'config.cpp', 'mupen64plusplus/plugin.c',
                        'wxvidext.cpp'] + osal_src + debugger_sources,
                target='wxmupen64plus',
                lib=additional_libs,
                uselib = 'SDL wxWidgets',
                install_path = bin_install_path,
                includes=['.', api_path])
