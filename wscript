APPNAME = 'gravity'
VERSION = '0.1'

def options(opt):
    opt.load('compiler_cxx')

    opt.add_option(
        '--release', action='store_true', default=False, dest='release_build',
        help='Build a release build.')

    opt.add_option(
        '--enable-profiling', action='store_true', default=False, dest='profiling_enabled',
        help='Enable collecting profiling information.'
    )

def configure(cfg):
    cfg.load('compiler_cxx')

    cfg.check_cfg(package='sdl2', args='--cflags --libs', uselib_store='SDL2')
    cfg.check_cxx(lib='Box2D', uselib_store='BOX2D')
    cfg.check_cxx(lib='SDL2_ttf', uselib_store='SDL2_TTF')
    cfg.check_cxx(lib='SDL2_mixer', uselib_store='SDL2_MIXER')
    cfg.check_cxx(lib='GL', uselib_store='GL')
    cfg.check_cxx(lib='GLEW', uselib_store='GLEW')
    cfg.check_cxx(lib='SOIL', uselib_store='SOIL')

    cfg.env.append_value('CXXFLAGS', ['-std=c++11'])

    if cfg.options.release_build:
        cfg.env.append_value('CXXFLAGS', ['-O3'])
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
        'splash-screen.cc',
        'game-screen.cc',
        'main-menu-screen.cc',
        'high-scores-screen.cc',
        'entity.cc',
        'resource-cache.cc',
        'helpers.cc',
        'config.cc',
        'image-widget.cc',
        'label-widget.cc',
        'button-widget.cc',
        'mesh.cc',
        'renderer.cc'
    ]
    bld.program(
        source=source,
        target='gravity',
        use='SDL2 SDL2_TTF SDL2_MIXER GL GLEW SOIL BOX2D'
    )
