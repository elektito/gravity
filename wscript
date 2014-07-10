APPNAME = 'gravity'
VERSION = '0.1'

def options(opt):
    opt.load('compiler_cxx')

    opt.add_option(
        '--release', action='store_true', default=False, dest='release_build',
        help='Build a release build.')

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

def build(bld):
    source = [
        'main.cc',
        'sdl-renderer.cc',
        'camera.cc',
        'timer.cc',
        'game-screen.cc',
        'main-menu-screen.cc',
        'high-scores-screen.cc',
        'entity.cc',
        'resource-cache.cc',
        'helpers.cc',
        'config.cc',
        'button-widget.cc'
    ]
    bld.program(
        source=source,
        target='gravity',
        use='SDL2 SDL2_TTF SDL2_MIXER GL GLEW SOIL BOX2D'
    )
