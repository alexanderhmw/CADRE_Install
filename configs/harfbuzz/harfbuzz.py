from MakeIt import ConfigFunc

name = "harfbuzz"
version_list = [2, 9, 1]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['libfreetype6-dev', 'libglib2.0-dev', 'libcairo2-dev', 'autoconf', 'automake', 'libtool', 'pkg-config', 'ragel', 'gtk-doc-tools', 'libgraphite2-dev'],
        'remove': ['libharfbuzz-dev']
    },
    "git": {
        'url': 'https://github.com/harfbuzz/harfbuzz.git',
        'branch': ConfigFunc.version_str(version_list, ".")
    },
    "params": {
        'src': '.',
        'type': 'script',
        'args': ['./autogen.sh', './configure --prefix=/usr'],
        'make': ['-j8']
    }
}

ConfigFunc.export_config_json(__file__, config)
