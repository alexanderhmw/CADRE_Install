from MakeIt import ConfigFunc
from configs.essentials.Qt import Qt
from pathlib import Path

name = "OpenSSL"
version_list = Qt.version_list

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "precmds": [
        {'cmd': 'cd /opt/Qt/Tools/OpenSSL/src && sudo ./config --prefix=/opt/Qt/Tools/OpenSSL && sudo make && sudo make install'},
    ],
    "ldpath": ['/opt/Qt/Tools/OpenSSL/lib'],
}

ConfigFunc.export_config_json(__file__, config)
