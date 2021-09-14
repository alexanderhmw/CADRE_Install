from MakeIt import ConfigFunc
import os

name = "dsrcasn"
version_list = [1, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'file': os.path.join(ConfigFunc.root_offline_pkgs, 'dsrcasn/dsrc_{}_amd64.deb'.format(ConfigFunc.version_str(version_list, "."))),
        'type': 'deb'
    }
}

ConfigFunc.export_config_json(__file__, config)
