from MakeIt import ConfigFunc
from configs.essentials.boost import boost

name = "gf_orb_slam2"
version_list = [1, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {'pkgs' : ['libgtest-dev', 'freeglut3', 'freeglut3-dev']},
    "git": {
        'url': 'https://github.com/ivalab/gf_orb_slam2.git',
        'branch': "binary_map_io",
        'patch' : ConfigFunc.to_absolute_path(['GF_ORB_SLAM2.patch'],
                                              ConfigFunc.resolve_file_path(__file__)),
    },
    "params": [
        {'src' : './Thirdparty/DBoW2', 'type' : 'cmake',
         'args' : ['..', '-DCMAKE_BUILD_TYPE=Release', '-DDBOW2_LIB_TYPE:STRING=STATIC',
                   '-DBOOST_ROOT:PATH=/opt/boost/{}'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_INCLUDEDIR:PATH=/opt/boost/{}/include'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_LIBRARYDIR:PATH=/opt/boost/{}/lib'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBoost_NO_SYSTEM_PATH:BOOL=ON'],
         'make' : ['-j8'], 'install' : None},
        {'src' : './Thirdparty/g2o', 'type' : 'cmake',
         'args' : ['..', '-DCMAKE_BUILD_TYPE=Release', '-DG2O_LIB_TYPE:STRING=STATIC',
                   '-DBOOST_ROOT:PATH=/opt/boost/{}'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_INCLUDEDIR:PATH=/opt/boost/{}/include'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_LIBRARYDIR:PATH=/opt/boost/{}/lib'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBoost_NO_SYSTEM_PATH:BOOL=ON'],
         'make' : ['-j8'], 'install' : None},
        {'src' : './Thirdparty/SLAM++', 'type' : 'cmake',
         'args' : ['..', '-DCMAKE_BUILD_TYPE=Release', '-DSLAM_P_P_EIGEN33:BOOL=ON', '-DSLAM_P_P_USE_OPENMP:BOOL=ON', '-DCXXFLAGS=-fpermissive',
                   '-DSLAM_P_P_FLAT_SYSTEM_ALIGNED_MEMORY:BOOL=OFF', '-DSLAM_P_P_LIB_TYPE:STRING=STATIC',
                   '-DBOOST_ROOT:PATH=/opt/boost/{}'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_INCLUDEDIR:PATH=/opt/boost/{}/include'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_LIBRARYDIR:PATH=/opt/boost/{}/lib'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBoost_NO_SYSTEM_PATH:BOOL=ON'],
         'make' : ['-j8'], 'install' : None},
        {'src' : '.', 'type' : 'cmake',
         'args' : ['..', '-DCMAKE_BUILD_TYPE=Release', '-DENABLE_CUDA_IN_OPENCV=False', '-DORB_LIB_TYPE:STRING=STATIC',
                   '-DBOOST_ROOT:PATH=/opt/boost/{}'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_INCLUDEDIR:PATH=/opt/boost/{}/include'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBOOST_LIBRARYDIR:PATH=/opt/boost/{}/lib'.format(ConfigFunc.version_str(boost.version_list, "_")),
                   '-DBoost_NO_SYSTEM_PATH:BOOL=ON'],
         'make' : ['-j8'], 'install' : None}
    ],
    "dependencies": ["boost", "armadillo", "Pangolin"]
}

ConfigFunc.export_config_json(__file__, config)
