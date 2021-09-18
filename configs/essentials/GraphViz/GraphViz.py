from MakeIt import ConfigFunc

name = "GraphViz"
version_list = [2, 49, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {'pkgs': ['bison', 'flex', 'automake', 'autoconf', 'libtool']},
    "pkg": {
        'url': 'https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/{}/graphviz-{}.tar.gz'
            .format(ConfigFunc.version_str(version_list, "."), ConfigFunc.version_str(version_list, ".")),
        'type': 'tar.gz'
    },
    "params": {
        'src': 'graphviz-{}'.format(ConfigFunc.version_str(version_list, ".")),
        'type': 'script',
        'args': ['./configure'],
        'make': ['-j8']
    }
}

ConfigFunc.export_config_json(__file__, config)
