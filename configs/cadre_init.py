from MakeIt import ConfigFunc

name = "cadre_init"
version_list = [1, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    # "precmds": [{'cmd' : 'echo "export UC_DIR={}" >> ~/.bashrc'.format(root), 'condition' : '! grep -q "export UC_DIR={}" ~/.bashrc'.format(root)},
    #             {'cmd' : 'echo \'export UC_CONFIG_DIR=${UC_DIR}/config\' >> ~/.bashrc', 'condition' : '! grep -q \'export UC_CONFIG_DIR=${UC_DIR}/config\' ~/.bashrc'},
    #             {'cmd' : 'echo \'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${UC_DIR}/lib\' >> ~/.bashrc', 'condition' : '! grep -q \'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${UC_DIR}/lib\' ~/.bashrc'},
    #             {'cmd' : 'echo \'export PATH=${UC_DIR}/bin:${UC_DIR}/scripts:${PATH}\' >> ~/.bashrc', 'condition' : '! grep -q \'export PATH=${UC_DIR}/bin:${UC_DIR}/scripts:${PATH}\' ~/.bashrc'}],
    "apt": {
        'pkgs': ['vim', 'build-essential', 'git', 'ssh', 'cmake', 'python3-dev', 'python3-pip'],
    },
}

ConfigFunc.export_config_json(__file__, config)
