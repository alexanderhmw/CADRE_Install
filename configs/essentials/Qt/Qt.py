from MakeIt import ConfigFunc
from pathlib import Path

name = "Qt"
version_list = [5, 15, 2]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['qt5-default'],
    },
    "ldpath": ['/opt/Qt/{}/gcc_64/lib'.format(ConfigFunc.version_str(version_list, "."))],
    "postcmds": [
        {'cmd': "qtchooser -install qt5-opt /opt/Qt/{}/gcc_64/bin/qmake".format(ConfigFunc.version_str(version_list, ".")), 'condition': "! (qtchooser -l | grep -q 'qt5-opt')"},
        {'cmd': "echo \"export QT_SELECT=qt5-opt\" >> ~/.bashrc", 'condition': '! grep -q "export QT_SELECT=qt5-opt" ~/.bashrc'},
    ]
}

ConfigFunc.export_config_json(__file__, config)
