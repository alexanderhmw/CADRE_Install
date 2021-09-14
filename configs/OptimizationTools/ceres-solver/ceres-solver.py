from MakeIt import ConfigFunc

name = "ceres-solver"
version_list = [2, 0, 0]

config ={
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['libgoogle-glog-dev', 'libgflags-dev']
    },
    "git": {
        'url': 'https://github.com/ceres-solver/ceres-solver.git',
        'branch': ConfigFunc.version_str(version_list, ".")
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args' : ['..',
                  '-DCMAKE_INSTALL_PREFIX=/opt/ceres-solver', '-DBUILD_SHARED_LIBS=ON', '-DCXX11=ON',
                  '-DEIGEN_INCLUDE_DIR=/opt/eigen/include/eigen', '-DEigen3_DIR=/opt/eigen/share/eigen/cmake',
                  '-DBLAS_openblas_LIBRARY=/opt/OpenBLAS/lib/libopenblas.so',
                  '-DMETIS_LIBRARY=/opt/SuiteSparse/lib/libmetis.so',
                  '-DAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DAMD_LIBRARY=/opt/SuiteSparse/lib/libamd.so',
                  '-DCAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCAMD_LIBRARY=/opt/SuiteSparse/lib/libcamd.so',
                  '-DCCOLAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCCOLAMD_LIBRARY=/opt/SuiteSparse/lib/libccolamd.so',
                  '-DCOLAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCOLAMD_LIBRARY=/opt/SuiteSparse/lib/libcolamd.so',
                  '-DCHOLMOD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCHOLMOD_LIBRARY=/opt/SuiteSparse/lib/libcholmod.so',
                  '-DCXSPARSE_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCXSPARSE_LIBRARY=/opt/SuiteSparse/lib/libcxsparse.so',
                  '-DSUITESPARSE=ON',
                  '-DSUITESPARSE_CONFIG_INCLUDE_DIR=/opt/SuiteSparse/include', '-DSUITESPARSE_CONFIG_LIBRARY=/opt/SuiteSparse/lib/libsuitesparseconfig.so',
                  '-DSUITESPARSEQR_INCLUDE_DIR=/opt/SuiteSparse/include', '-DSUITESPARSEQR_LIBRARY=/opt/SuiteSparse/lib/libspqr.so'],
        'make': ['-j8']
    },
    "ldpath": ['/opt/ceres-solver/lib'],
    "dependencies": ["eigen3", "OpenBLAS", "SuiteSparse"]
}

ConfigFunc.export_config_json(__file__, config)
