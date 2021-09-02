from MakeIt import ConfigFunc
from configs.boost import boost

name = "COSMOSLibrary"
version_list = [1, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['libcurl4-openssl-dev']
    },
    "pkg": {
        'file': ConfigFunc.to_absolute_path('COSMOSLibrary.tar.xz', ConfigFunc.resolve_file_path(__file__)),
        'type': 'tar.xz'
    },
    "params": {
        'src': 'src',
        'type': 'cmake',
        'args': ['..',
                 '-DBOOST_ROOT:PATH=/opt/boost/{}'.format(ConfigFunc.version_str(boost.version_list, "_")),
                 '-DBOOST_INCLUDEDIR:PATH=/opt/boost/{}/include'.format(ConfigFunc.version_str(boost.version_list, "_")),
                 '-DBOOST_LIBRARYDIR:PATH=/opt/boost/{}/lib'.format(ConfigFunc.version_str(boost.version_list, "_")),
                 '-DBoost_NO_SYSTEM_PATH:BOOL=ON'],
        'make': ['-j8']
    },
    "dependencies": ["boost"]
}

ConfigFunc.export_config_json(__file__, config)
