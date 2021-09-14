from MakeIt import ConfigFunc
import os

name = "pdk"
version_list = [0, 1, 272]
subversion = 'alpha'

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'file': os.path.join(ConfigFunc.root_offline_pkgs, 'pdk/pdk_{}-{}_amd64.deb'.format(ConfigFunc.version_str(version_list, "."), subversion)),
        'type': 'deb'
    },
    "dependencies": ["pdk-common"]
}

ConfigFunc.export_config_json(__file__, config)
