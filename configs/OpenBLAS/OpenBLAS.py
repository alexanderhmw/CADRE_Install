from MakeIt import ConfigFunc

name = "OpenBLAS"
version_list = [0, 3, 17]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['libblas-dev', 'liblapack-dev'],
        'remove': ['libopenblas-dev', 'libopenblas-base']
    },
    "git": {
        'url': 'https://github.com/xianyi/OpenBLAS.git',
        'branch': 'v{}'.format(ConfigFunc.version_str(version_list, "."))
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..', '-DCMAKE_INSTALL_PREFIX=/opt/OpenBLAS', '-DBUILD_SHARED_LIBS=ON'],
        'make': ['USE_THREAD=0', 'DYNAMIC_ARCH=1', '-j8']
    },
    "ldpath": ['/opt/OpenBLAS/lib'],
    "alternatives": [{'link': '/usr/lib/libblas.so.3', 'name': 'libblas.so.3', 'path': '/opt/OpenBLAS/lib/libopenblas.so', 'priority': '41'}]
}

ConfigFunc.export_config_json(__file__, config)
