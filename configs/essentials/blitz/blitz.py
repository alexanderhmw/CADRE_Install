from MakeIt import ConfigFunc

name = "blitz"
version_list = [1, 0, 2]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://github.com/blitzpp/blitz.git',
        'branch': ConfigFunc.version_str(version_list, ".")
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..'],
        'make': ['lib', 'CXXFLAGS="-std=c++11"', 'MAKEINFO=true', '-j8'],
        'install': ['CXXFLAGS="-std=c++11"', 'MAKEINFO=true']
    }
}

ConfigFunc.export_config_json(__file__, config)
