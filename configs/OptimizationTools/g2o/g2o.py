from MakeIt import ConfigFunc

name = "g2o"
version_list = [20201223]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {'pkgs': ['libqglviewer-dev-qt5']},
    "git": {
        'url': 'https://github.com/RainerKuemmerle/g2o.git',
        'branch': '{}_git'.format(ConfigFunc.version_str(version_list, "."))
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..',
                 '-DCMAKE_INSTALL_PREFIX=/opt/g2o', '-DBUILD_SHARED_LIBS=ON',
                 '-DEIGEN3_INCLUDE_DIR=/opt/eigen/include/eigen3',
                 '-DBLAS_openblas_LIBRARY=/opt/OpenBLAS/lib/libopenblas.so',
                 '-DCHOLMOD_METIS_LIBRARY=/opt/SuiteSparse/lib/libmetis.so',
                 '-DCHOLMOD_SUITESPARSECONFIG_LIBRARY=/opt/SuiteSparse/lib/libsuitesparseconfig.so',
                 '-DAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DAMD_LIBRARY=/opt/SuiteSparse/lib/libamd.so',
                 '-DCAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCAMD_LIBRARY=/opt/SuiteSparse/lib/libcamd.so',
                 '-DCCOLAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCCOLAMD_LIBRARY=/opt/SuiteSparse/lib/libccolamd.so',
                 '-DCOLAMD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCOLAMD_LIBRARY=/opt/SuiteSparse/lib/libcolamd.so',
                 '-DCHOLMOD_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCHOLMOD_LIBRARY=/opt/SuiteSparse/lib/libcholmod.so',
                 '-DCSPARSE_INCLUDE_DIR=/opt/SuiteSparse/include', '-DCSPARSE_LIBRARY=/opt/SuiteSparse/lib/libcxsparse.so',
                 '-DG2O_USE_OPENMP=ON',
                 '-DQt5_DIR=' + ConfigFunc.getQTDIR() + '/lib/cmake/Qt5',
                 '-DQt5Core_DIR=' + ConfigFunc.getQTDIR() + '/lib/cmake/Qt5Core',
                 '-DQt5Gui_DIR=' + ConfigFunc.getQTDIR() + '/lib/cmake/Qt5Gui',
                 '-DQt5OpenGL_DIR=' + ConfigFunc.getQTDIR() + '/lib/cmake/Qt5OpenGL',
                 '-DQt5Widgets_DIR=' + ConfigFunc.getQTDIR() + '/lib/cmake/Qt5Widgets',
                 '-DQt5Xml_DIR=' + ConfigFunc.getQTDIR() + '/lib/cmake/Qt5Xml'],
        'make': ['-j8']
    },
    "ldpath": ['/opt/g2o/lib'],
    "dependencies": ["eigen3", "OpenBLAS", "SuiteSparse"]
}

ConfigFunc.export_config_json(__file__, config)
