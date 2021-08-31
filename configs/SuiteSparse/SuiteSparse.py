from MakeIt import ConfigFunc

name = "SuiteSparse"
version_list = [5, 10, 1]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['libblas-dev', 'liblapack-dev'],
        'remove' : ['libopenblas-dev', 'libopenblas-base']
    },
    "git": {
        'url': 'https://github.com/DrTimothyAldenDavis/SuiteSparse.git',
        'branch': 'v{}'.format(ConfigFunc.version_str(version_list, ".")),
        'patch': ConfigFunc.to_absolute_path(["./SuiteSparse.patch"])
    },
    "params": {
        'src': '.',
        'make': ['library', 'CUDA_PATH=/usr/local/cuda', 'BLAS=/opt/OpenBLAS/lib/libopenblas.so', '-j8'],
        'install': ['INSTALL=/opt/SuiteSparse', 'CUDA_PATH=/usr/local/cuda', 'BLAS=/opt/OpenBLAS/lib/libopenblas.so']
    },
    "ldpath": ['/opt/SuiteSparse/lib'],
    "dependencies": ["OpenBLAS", "cuda"]
}

ConfigFunc.export_config_json(__file__, config)
