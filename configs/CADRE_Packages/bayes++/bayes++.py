from MakeIt import ConfigFunc
import os

name = "bayes++"
version_list = [2010, 8, 1]
subversion = 2

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'file': os.path.join(ConfigFunc.root_offline_pkgs, 'bayes++/bayes++-dev_{}-{}_amd64.deb'.format(ConfigFunc.version_str(version_list, "."), subversion)),
        'type': 'deb'
    }
}

ConfigFunc.export_config_json(__file__, config)
