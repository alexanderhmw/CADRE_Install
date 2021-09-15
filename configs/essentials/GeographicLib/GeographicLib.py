from MakeIt import ConfigFunc

name = "GeographicLib"
version = "release"

config = {
    "name": name,
    "version": version,
    "git": {
        'url': 'https://git.code.sf.net/p/geographiclib/code',
        'branch' : version
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..'],
        'make': ['-j8']
    }
}

ConfigFunc.export_config_json(__file__, config)
