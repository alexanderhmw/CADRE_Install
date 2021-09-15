from MakeIt import ConfigFunc

name = "Pangolin"
version_list = [0, 6]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['libglew-dev']
    },
    "git": {
        'url': 'https://github.com/stevenlovegrove/Pangolin.git',
        'branch': "v{}".format(ConfigFunc.version_str(version_list, "."))
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..',
                 '-DCMAKE_INSTALL_PREFIX:PATH=/opt/Pangolin',
                 '-DCMAKE_BUILD_TYPE:STRING=Release',
                 '-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
                 '-DEIGEN3_INCLUDE_DIR:PATH=/opt/eigen/include/eigen3',
                 '-DJPEG_INCLUDE_DIR=/opt/libjpeg-turbo/include',
                 '-DJPEG_LIBRARY_RELEASE=/opt/libjpeg-turbo/lib64/libjpeg.so'],
        'make': ['-j8']
    },
    "ldpath": ['/opt/Pangolin/lib'],
    "dependencies": ["eigen3", "libjpeg-turbo"]
}

ConfigFunc.export_config_json(__file__, config)
