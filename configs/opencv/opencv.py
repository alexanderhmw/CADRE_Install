from MakeIt import ConfigFunc

name = "opencv"
version_list = [3, 4, 7]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "apt": {
        'pkgs': ['build-essential', 'cmake', 'git', 'libgtk2.0-dev', 'pkg-config', 'libavcodec-dev', 'libavformat-dev', 'libswscale-dev',
                 'python-dev', 'python-numpy', 'libtbb2', 'libtbb-dev', 'libjpeg-dev', 'libpng-dev', 'libtiff-dev', 'libjasper-dev', 'libdc1394-22-dev']
    },
    "git": {
        'url': 'https://github.com/opencv/opencv.git',
        'branch': ConfigFunc.version_str(version_list, ".")
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..',
                 '-DCMAKE_INSTALL_PREFIX=/opt/opencv', '-DBUILD_SHARED_LIBS=ON', '-DCMAKE_BUILD_TYPE=Release', '-DENABLE_CXX11=ON',
                 '-DCeres_DIR=/opt/ceres-solver/lib/cmake/Ceres',
                 '-DEIGEN_INCLUDE_DIR=/opt/eigen/include/eigen', '-DEIGEN_INCLUDE_PATH=/opt/eigen/include/eigen',
                 '-DWITH_CUDA:BOOL=ON', #'-DBUILD_opencv_cudacodec=OFF',
                 '-DBUILD_opencv_python2=ON', '-DPYTHON2_NUMPY_INCLUDE_DIRS=/usr/local/lib/python2.7/dist-packages/numpy/core/include',
                 '-DBUILD_opencv_python3=ON', '-DPYTHON3_NUMPY_INCLUDE_DIRS=/usr/local/lib/python3.5/dist-packages/numpy/core/include',
                 '-DWITH_QT:BOOL=ON', '-DWITH_OPENCL=OFF',
                 '-DBUILD_JPEG=OFF', '-DWITH_JPEG=ON',
                 '-DJPEG_INCLUDE_DIR=/opt/libjpeg-turbo/include',
                 '-DJPEG_LIBRARY_RELEASE=/opt/libjpeg-turbo/lib64/libjpeg.so',
                 '-DLAPACK_IMPL=OpenBLAS', '-DLAPACK_INCLUDE_DIR=/opt/OpenBLAS/include', '-DLAPACK_LIBRARIES=/opt/OpenBLAS/lib/libopenblas.so',
                 '-DOpenBLAS_INCLUDE_DIR=/opt/OpenBLAS/include', '-DOpenBLAS_LIB=/opt/OpenBLAS/lib/libopenblas.so',
                 '-DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules'],
        'make': ['-j8']
    },
    "ldpath": ['/opt/opencv/lib'],
    "dependencies": ["opencv_contrib", "ceres-solver", "eigen", "libjpeg-turbo", "OpenBLAS", "cuda"]
}

ConfigFunc.export_config_json(__file__, config)
