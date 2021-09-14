from MakeIt import ConfigFunc

name = "nmealib"
version_list = [0, 5, 3]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://github.com/Paulxia/nmealib.git',
        'branch': "v{}".format(ConfigFunc.version_str(version_list, ".")),
        'patch': ConfigFunc.to_absolute_path(["nmealib.patch"], ConfigFunc.resolve_file_path(__file__))
    },
    "params": {
        'src': '.',
        'make': ['all', 'DESTDIR=/opt/nmealib', 'CXXFLAGS="-std=c++11"'],
        'install': None,
    }
}

ConfigFunc.export_config_json(__file__, config)
