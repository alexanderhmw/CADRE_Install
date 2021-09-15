from MakeIt import ConfigFunc

name = "ads"
version_list = [3, 7, 2]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git',
        'branch': ConfigFunc.version_str(version_list, "."),
    },
    "params": [
        {'src': '.',
         'type': 'qmake',
         'args': ['-r', 'PREFIX=/opt/ads'],
         'make': ['-j8']}
    ],
    "ldpath": ['/opt/ads/lib']
}

ConfigFunc.export_config_json(__file__, config)
