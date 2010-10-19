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
    opt.add_option('--mupenapi', action='store', help='Where to find Mupen64Plus API headers (optional)', default='/usr/include')
    opt.add_option('--wxconfig', action='store', help='Which wx-config utility to use (optional)', default='wx-config', dest="wxconfig")
    opt.add_option('--sdlconfig', action='store', help='Which sdl-config utility to use (optional)', default='sdl-config', dest="sdlconfig")
    opt.add_option('--debug', action='store', help='Whether to make a debug build', default=False, dest="debugmode")
    opt.load('compiler_cxx')
    opt.load('compiler_c')

# --------------------------------------------------------------------------------------------
#                                          CONFIGURE
# --------------------------------------------------------------------------------------------

def configure(ctx):
    import Options
    import subprocess
    
    api_path = Options.options.mupenapi
    wx_config = Options.options.wxconfig
    sdl_config = Options.options.sdlconfig
    is_debug = Options.options.debugmode
    
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    
    ctx.env['api_path'] = api_path
    ctx.env['is_debug'] = is_debug
    
    ctx.find_program('gcc', var='GCC', mandatory=True)
    ctx.find_program('g++', var='GPP', mandatory=True)
    ctx.check_cc(header_name="stdio.h", function_name='printf', msg='Checking C compiler works', errmsg="No")
    ctx.check_cxx(header_name="string", function_name='printf', msg='Checking C++ compiler works', errmsg="No")
    ctx.check_cc(header_name="m64p_frontend.h", includes=[api_path])
    ctx.check_cc(header_name="m64p_config.h", includes=[api_path])
    ctx.check_cc(header_name="m64p_types.h", includes=[api_path])
    
    ctx.check_cfg(path=sdl_config, args='--cflags --libs', package='', uselib_store='SDL')
    
    # FIXME: Work around a bug in Waf. wx-config is checked for but the result is unused; and
    #        instead I manually get the build flags through "subprocess.check_output"
    ctx.check_cfg(path=wx_config, args='--cxxflags --libs', package='', uselib_store='wxWidgets')
    ctx.env['wxCppFlags'] = subprocess.check_output(wx_config.split() + ["--cppflags","core,base,gl"])
    ctx.env['wxLdFlags']  = subprocess.check_output(wx_config.split() + ["--libs", "core,base,gl"])

# --------------------------------------------------------------------------------------------
#                                            BUILD
# --------------------------------------------------------------------------------------------

def build(bld):
    api_path = bld.env['api_path']

    print(bld.env['wxCppFlags'])
    print(bld.env['wxLdFlags'])

    link_flags = []
    build_flags = []
    framework_list = [] # for OS X
    
    build_flags += bld.env['wxCppFlags'].split()
    
    # FIXME: hack to extract frameworks out of link flags, see above FIXME, we can't let waf
    #        parse the output of wx-config... I need to make a bug report to waf, get
    #        check_cfg fixed, then this whole hack can be removed
    temp_ld_flags = bld.env['wxLdFlags'].split()
    fw = False
    for n in temp_ld_flags:
        if n == "-framework":
            fw = True
        elif fw:
            framework_list.append(n)
            link_flags.append("-Wl,-framework,"+n)
            fw = False
        else:
            link_flags.append(n)
    
    if bld.env['is_debug']:
        build_flags += ['-g']
    else:
        build_flags += ['-O2']

    bld.program(features='c cxx cxxprogram',
                cxxflags=build_flags,
                cflags=build_flags,
                linkflags=link_flags,
                source=['main.cpp', 'gamespanel.cpp', 'parameterpanel.cpp', 'sdlkeypicker.cpp',
                        'mupen64plusplus/MupenAPIpp.cpp', 'mupen64plusplus/MupenAPI.c',
                        'mupen64plusplus/osal_dynamiclib_unix.c',
                        'mupen64plusplus/osal_files_unix.c', 'mupen64plusplus/plugin.c'],
                target='wxMupen64Plus',
                uselib = 'SDL',
                framework=framework_list,
                includes=['.', api_path])
    
    data_dir = bld.path.find_dir('data')
    bld.install_files('${PREFIX}/share/wxmupen64plus/', data_dir.ant_glob('*'))
