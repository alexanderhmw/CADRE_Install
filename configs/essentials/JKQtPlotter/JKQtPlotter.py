from MakeIt import ConfigFunc
from configs.essentials.Qt import Qt

name = "JKQtPlotter"
version_list = [2019, 11, 3]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'remove': ['qt5-qmake']
    },
    "git": {
        'url': 'https://github.com/jkriege2/JKQtPlotter.git',
        'branch': "master",
    },
    "params": [
        {'src': '.',
        'type': 'cmake',
        'args': ['..',
                 '-DCMAKE_PREFIX_PATH={}'.format(ConfigFunc.getQTDIR()),
                 '-DQt5_DIR={}/lib/cmake/Qt5'.format(ConfigFunc.getQTDIR()),
                 '-DCMAKE_INSTALL_PREFIX=/opt/JKQtPlotter'],
        'make': ['-j8']}
    ],
    "ldpath": ['/opt/JKQtPlotter/lib'],
    "dependencies": ["Qt"]
}

ConfigFunc.export_config_json(__file__, config)
