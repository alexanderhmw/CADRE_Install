from MakeIt import ConfigFunc
import os

name = "ipc"
version_list = [3, 7, 10]
subversion = 4

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'file': os.path.join(ConfigFunc.root_offline_pkgs, 'ipc/ipc_{}-{}_amd64.deb'.format(ConfigFunc.version_str(version_list, "."), subversion)),
        'type': 'deb'
    }
}

ConfigFunc.export_config_json(__file__, config)
