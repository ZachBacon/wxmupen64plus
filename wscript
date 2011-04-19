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
VERSION='0.1'
APPNAME='wxMupen64Plus'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

# --------------------------------------------------------------------------------------------
#                                           OPTIONS
# --------------------------------------------------------------------------------------------

def options(opt):
    import os
    
    opt.add_option('--mupenapi',  action='store', help='Where to find Mupen64Plus API headers (optional)', default='/usr/include')
    opt.add_option('--wxconfig',  action='store', help='Which wx-config utility to use (optional)', default='wx-config', dest="wxconfig")
    opt.add_option('--sdlconfig', action='store', help='Which sdl-config utility to use (optional)', default='sdl-config', dest="sdlconfig")
    opt.add_option('--debug',     action='store', help='Whether to make a debug build (may be \'true\' or \'false\')', default='false', dest="debugmode")
    opt.add_option('--wxconfig_args', action='store', help='Additional arguments passed to wx-config', default='',  dest='wxconfig_args')
    
    if os.name == 'nt':
        opt.add_option('--wxhome', action='store', help='Where your wxWidgets build is installed', default=None,  dest='wxhome')

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
    import Options
    import subprocess
    import waflib
    import os
    
    api_path   = Options.options.mupenapi
    wx_config  = Options.options.wxconfig
    sdl_config = Options.options.sdlconfig
    
    wxconfig_args = Options.options.wxconfig_args
    
    wxhome = ''
    if os.name == 'nt':
        wxhome = ''.join(Options.options.wxhome)
    
    if Options.options.debugmode != 'true' and Options.options.debugmode != 'false':
        waflib.Logs.warn("Warning, the --debug option may only be 'true' or 'false'. Defaulting to 'false'.")
    
    is_debug   = (Options.options.debugmode == 'true')
    
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    
    ctx.env['api_path'] = api_path
    ctx.env['is_debug'] = is_debug
    ctx.env['wxhome'] = wxhome
    
    ctx.find_program('gcc', var='GCC', mandatory=True)
    ctx.find_program('g++', var='GPP', mandatory=True)
    ctx.check_cc(header_name="stdio.h", function_name='printf', msg='Checking C compiler works', errmsg="No")
    ctx.check_cxx(header_name="cstdio", function_name='printf', msg='Checking C++ compiler works', errmsg="No")
    ctx.check_cc(header_name="m64p_frontend.h", includes=[api_path])
    ctx.check_cc(header_name="m64p_config.h",   includes=[api_path])
    ctx.check_cc(header_name="m64p_types.h",    includes=[api_path])
    
    ctx.check_cfg(path=sdl_config, args='--cflags --libs',   package='', uselib_store='SDL')
    
    if os.name == 'nt':
        if wxhome == None :
            ctx.fatal("On Windows, the --wxhome argument is mandatory")
        ctx.check_cfg(msg="Checking for wxWidgets 2.9.x", path=wx_config,  args='--version=2.9 --cxxflags --prefix=' + wxhome + ' ' + wxconfig_args + ' --libs core,base,gl,html', package='', uselib_store='wxWidgets')
    else:
        ctx.check_cfg(msg="Checking for wxWidgets 2.9.x", path=wx_config,  args='--version=2.9 --cxxflags --libs core,base,gl,html ' + wxconfig_args, package='', uselib_store='wxWidgets')

# --------------------------------------------------------------------------------------------
#                                            BUILD
# --------------------------------------------------------------------------------------------

def build(bld):
    import os

    api_path = bld.env['api_path']

    wxhome = bld.env['wxhome']
    
    link_flags = []
    build_flags = []
    
    # Set up debug or release build options
    if bld.env['is_debug']:
        build_flags += ['-g','-DDEBUG=1']
    else:
        build_flags += ['-O2','-DNEBUG=1']

    bin_install_path = "${PREFIX}/bin"

    osal_src = []
    additional_links = []
    
    # Windows
    if os.name == 'nt':
        cmd = "windres --include-dir=" + wxhome + r"\include ${SRC} --output build/${TGT}"
        bld(rule=cmd, source='wxmupen64plus.rc', target='manifest.o')
        
        osal_src += ['mupen64plusplus/osal_dynamiclib_win32.c', 'mupen64plusplus/osal_files_win32.c']
        additional_links += ['manifest.o']
        
    # A few OSX-specific flags
    elif os.uname()[0] == 'Darwin':
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
        
    else:
        # For other unices
        osal_src += ['mupen64plusplus/osal_dynamiclib_unix.c', 'mupen64plusplus/osal_files_unix.c']
        
        # install target
        data_dir = bld.path.find_dir('data')
        bld.install_files('${PREFIX}/share/wxmupen64plus/', data_dir.ant_glob('*'))


    # Build the program
    bld.program(features='c cxx cxxprogram',
                cxxflags=build_flags,
                cflags=build_flags+['-Wfatal-errors'],
                linkflags=link_flags + additional_links,
                source=['main.cpp', 'gamespanel.cpp', 'parameterpanel.cpp', 'sdlkeypicker.cpp',
                        'mupen64plusplus/MupenAPIpp.cpp', 'mupen64plusplus/MupenAPI.c',
                        'sdlhelper.cpp', 'config.cpp', 'mupen64plusplus/plugin.c',
                        'wxvidext.cpp'] + osal_src,
                target='wxmupen64plus',
                uselib = 'SDL wxWidgets',
                install_path = bin_install_path,
                includes=['.', api_path])
