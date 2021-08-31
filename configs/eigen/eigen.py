from MakeIt import ConfigFunc

name = "eigen"
version_list = [3, 4, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://gitlab.com/libeigen/eigen.git',
        'branch': ConfigFunc.version_str(version_list, "."),
        'patch': ConfigFunc.to_absolute_path(["./eigen.patch"])
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..', '-DCMAKE_INSTALL_PREFIX:PATH=/opt/eigen', '-DCMAKE_BUILD_TYPE:STRING=Release'],
        'make': None,
    }
}

ConfigFunc.export_config_json(__file__, config)
