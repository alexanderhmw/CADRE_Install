from MakeIt import ConfigFunc

name = "LLVM"
version_list = [12, 0, 1]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://github.com/llvm/llvm-project.git',
        'branch': 'llvmorg-{}'.format(ConfigFunc.version_str(version_list, "."))
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['../llvm', '-DCMAKE_BUILD_TYPE=Release', '-DLLVM_ENABLE_RTTI=ON', '-DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra"', '-DCMAKE_INSTALL_PREFIX=/opt/LLVM_Clang'],
        'make' : ['-j8']
    },
    "ldpath": ['/opt/LLVM_Clang/lib'],
    "postcmds": [{'cmd' : 'echo "export LLVM_INSTALL_DIR=/opt/LLVM_Clang" >> ~/.bashrc', 'condition' : '! grep -q "export LLVM_INSTALL_DIR=/opt/LLVM_Clang" ~/.bashrc'},
                {'cmd' : 'echo \'export PATH=${PATH}:/opt/LLVM_Clang/bin\' >> ~/.bashrc', 'condition' : '! grep -q \'export PATH=${PATH}:/opt/LLVM_Clang/bin\' ~/.bashrc'}]
}

ConfigFunc.export_config_json(__file__, config)
