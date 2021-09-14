from MakeIt import ConfigFunc

name = "armadillo"
version_list = [10, 7]
subversion = 'x'

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://gitlab.com/conradsnicta/armadillo-code.git',
        'branch': "{}.{}".format(ConfigFunc.version_str(version_list, "."), subversion)
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..',
                 '-DCMAKE_INSTALL_PREFIX:PATH=/opt/armadillo',
                 '-DCMAKE_INSTALL_LIBDIR:PATH=lib',
                 '-DCMAKE_BUILD_TYPE:STRING=Release',
                 '-Dopenblas_LIBRARY:FILEPATH=/opt/OpenBLAS/lib/libopenblas.so',
                 '-Dopenblasp_LIBRARY:FILEPATH=/opt/OpenBLAS/lib/libopenblas.so',
                 '-Dopenblaso_LIBRARY:FILEPATH=/opt/OpenBLAS/lib/libopenblas.so',
                 '-DLAPACK_LIBRARY:FILEPATH=/opt/OpenBLAS/lib/libopenblas.so'],
        'make': ['-j8']
    },
    "ldpath": ['/opt/armadillo/lib'],
    "dependencies": ["OpenBLAS"]
}

ConfigFunc.export_config_json(__file__, config)
