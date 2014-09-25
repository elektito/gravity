APPNAME = 'gravity'
VERSION = '0.1'

from waflib.Task import Task

class PythonScript(Task):
    def __init__(self, env, args=''):
        Task.__init__(self, env=env)
        self.args = args

    def run(self):
        return self.exec_command("python {} {} {}".format(
            self.inputs[0].abspath(),
            self.outputs[0].abspath(),
            self.args
        ))

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
    bld.program(
        source=source,
        target='gravity-bin',
        use='SDL2 SDL2_TTF SDL2_MIXER GL BOX2D'
    )

    launcher = PythonScript(env=bld.env, args=bld.env.PREFIX + ' /share/gravity')
    launcher.set_inputs(bld.path.find_resource('create-launcher.py'))
    launcher.set_outputs(bld.path.find_or_declare('launcher.sh'))
    bld.add_to_group(launcher)

    bld.install_as('${PREFIX}/bin/gravity', 'launcher.sh', chmod=0755)

    images_dir = bld.path.find_dir('resources/images')
    bld.install_files('${PREFIX}/share/gravity/images', images_dir.ant_glob('*.png'), cwd=images_dir, relative_trick=True)

    fonts_dir = bld.path.find_dir('resources/fonts')
    bld.install_files('${PREFIX}/share/gravity/fonts', fonts_dir.ant_glob('*.ttf'), cwd=fonts_dir, relative_trick=True)

    sounds_dir = bld.path.find_dir('resources/sound')
    bld.install_files('${PREFIX}/share/gravity/sound', sounds_dir.ant_glob('*.wav'), cwd=sounds_dir, relative_trick=True)

    shaders_dir = bld.path.find_dir('resources/shaders')
    bld.install_files('${PREFIX}/share/gravity/shaders', shaders_dir.ant_glob('*.glsl'), cwd=shaders_dir, relative_trick=True)
