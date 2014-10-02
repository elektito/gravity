APPNAME = 'gravity'
VERSION = '1.0'

from waflib.Task import Task

def options(opt):
    opt.load('compiler_cxx')

    opt.add_option(
        '--release', action='store_true', default=False, dest='release_build',
        help='Build a release build.')

    opt.add_option(
        '--enable-profiling', action='store_true', default=False, dest='profiling_enabled',
        help='Enable collecting profiling information.'
    )

    opt.add_option(
        '--windows', action='store_true', default=False, dest='windows_build',
        help='Configure the build for Windows.'
    )

    opt.add_option(
        '--installer', action='store_true', default=False, dest='create_installer',
        help='Create a Windows installer using NSIS.'
    )

def configure(cfg):
    cfg.load('compiler_cxx')

    cfg.check_cfg(package='sdl2', args='--cflags --libs', uselib_store='SDL2')
    cfg.check_cxx(lib='Box2D', uselib_store='BOX2D')
    cfg.check_cfg(package='SDL2_ttf', args='--cflags --libs', uselib_store='SDL2_TTF')
    cfg.check_cxx(lib='SDL2_mixer', uselib_store='SDL2_MIXER')
    if cfg.options.windows_build:
        cfg.env['windows_build'] = True
        cfg.check_cxx(lib='opengl32', uselib_store='GL')
        cfg.check_cxx(lib='glu32', uselib_store='GLU')
        cfg.env.append_value('CXXFLAGS', '-DGLEW_STATIC')
        cfg.load('winres')

        if cfg.options.create_installer:
            cfg.env['create_installer'] = True
            cfg.find_program('makensis', var='MAKENSIS')
    else:
        if cfg.options.create_installer:
            raise cfg.errors.ConfigurationError('--installer options only available when --windows is used.')

        cfg.check_cxx(lib='GL', uselib_store='GL')

    cfg.env.append_value('CXXFLAGS', ['-std=c++11', '-DGLEW_NO_GLU'])

    if cfg.options.release_build:
        cfg.env.append_value('CXXFLAGS', ['-O3'])
        cfg.env.append_value('LINKFLAGS', '-Wl,-s')
        cfg.env.append_value('DEFINES', 'RELEASE_BUILD')
    else:
        cfg.env.append_value('CXXFLAGS', ['-g'])

    if cfg.options.profiling_enabled:
        cfg.env.append_value('CXXFLAGS', ['-pg'])
        cfg.env.append_value('LINKFLAGS', ['-pg'])

def build(bld):
    source = [
        'main.cc',
        'camera.cc',
        'timer.cc',
        'credits-screen.cc',
        'splash-screen.cc',
        'game-screen.cc',
        'main-menu-screen.cc',
        'high-scores-screen.cc',
        'entity.cc',
        'resource-cache.cc',
        'helpers.cc',
        'config.cc',
        'number-widget.cc',
        'image-widget.cc',
        'image-button-widget.cc',
        'label-widget.cc',
        'button-widget.cc',
        'mesh.cc',
        'renderer.cc',
        'glew.c'
    ]

    if bld.env.windows_build:
        source.append('windows/windows.cc')
        source.append('windows/resources.rc')
    else:
        source.append('posix/posix.cc')

    bld.program(
        source=source,
        target='gravity-bin',
        use='SDL2 SDL2_TTF SDL2_MIXER GL BOX2D'
    )

    if bld.env.create_installer:
        bld(rule='${MAKENSIS} -NOCD ${SRC}', source='windows/installer.nsis', target='gravity-installer.exe')

    if not bld.env.windows_build:
        bld(
            rule='echo "#!/bin/sh\\n{0}/bin/gravity-bin {0}/share/gravity"'.format(bld.env.PREFIX) + ' > ${TGT}',
            target='launcher.sh'
        )

        bld.install_as('${PREFIX}/bin/gravity', 'launcher.sh', chmod=0755)

    bld.install_as('${PREFIX}/share/doc/gravity/copyright', 'debian/copyright')

    images_dir = bld.path.find_dir('resources/images')
    bld.install_files('${PREFIX}/share/gravity/images', images_dir.ant_glob('*.png'), cwd=images_dir, relative_trick=True)

    fonts_dir = bld.path.find_dir('resources/fonts')
    bld.install_files('${PREFIX}/share/gravity/fonts', fonts_dir.ant_glob('*.ttf'), cwd=fonts_dir, relative_trick=True)

    sounds_dir = bld.path.find_dir('resources/sound')
    bld.install_files('${PREFIX}/share/gravity/sound', sounds_dir.ant_glob('*.wav'), cwd=sounds_dir, relative_trick=True)

    shaders_dir = bld.path.find_dir('resources/shaders')
    bld.install_files('${PREFIX}/share/gravity/shaders', shaders_dir.ant_glob('*.glsl'), cwd=shaders_dir, relative_trick=True)
