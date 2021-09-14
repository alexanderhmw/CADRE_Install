from MakeIt import ConfigFunc

name = "libjpeg-turbo"
version_list = [2, 1, 1]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://github.com/libjpeg-turbo/libjpeg-turbo.git',
        'branch': ConfigFunc.version_str(version_list, ".")
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..', '-DCMAKE_INSTALL_PREFIX=/opt/libjpeg-turbo'],
        'make': ['-j8']
    }
}

ConfigFunc.export_config_json(__file__, config)
