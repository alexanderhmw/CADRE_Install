from MakeIt import ConfigFunc
import os

name = "pdk-common"
version_list = [0, 1, 272]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'file': os.path.join(ConfigFunc.root_offline_pkgs, 'pdk/pdk-common_{}_amd64.deb'.format(ConfigFunc.version_str(version_list, "."))),
        'type': 'deb'
    }
}

ConfigFunc.export_config_json(__file__, config)
