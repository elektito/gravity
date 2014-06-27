APPNAME = 'gravity'
VERSION = '0.1'

def options(opt):
    opt.load('compiler_cxx')

def configure(cfg):
    cfg.load('compiler_cxx')

    cfg.check_cfg(package='sdl2', args='--cflags --libs', uselib_store='SDL2')
    cfg.check_cxx(lib='Box2D', uselib_store='BOX2D')
    cfg.check_cxx(lib='SDL2_gfx', uselib_store='SDL2_GFX')
    cfg.check_cxx(lib='SDL2_ttf', uselib_store='SDL2_TTF')

    cfg.env.append_value('CXXFLAGS', ['-std=c++11', '-g'])

def build(bld):
    source = [
        'main.cc',
        'sdl-renderer.cc',
        'camera.cc',
        'timer.cc',
        'game-screen.cc',
        'high-scores-screen.cc',
        'entity.cc',
        'font-cache.cc',
        'helpers.cc',
        'config.cc',
        'button-widget.cc'
    ]
    bld.program(
        source=source,
        target='gravity',
        use='SDL2 SDL2_GFX SDL2_TTF BOX2D'
    )
