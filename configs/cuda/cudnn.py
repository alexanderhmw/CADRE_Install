from MakeIt import ConfigFunc

name = "cudnn"
version_list = [8, 2, 2, 26]
cuda_version_list = [11, 4]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'url': 'https://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu2004/x86_64/nvidia-machine-learning-repo-ubuntu2004_1.0.0-1_amd64.deb',
        'type': 'deb',
        'postcmds': ['sudo apt-get update',
                     'sudo apt-get install libcudnn8={}-1+cuda{} -y'.format(ConfigFunc.version_str(version_list, "."), ConfigFunc.version_str(cuda_version_list, ".")),
                     'sudo apt-get install libcudnn8-dev={}-1+cuda{} -y'.format(ConfigFunc.version_str(version_list, "."), ConfigFunc.version_str(cuda_version_list, "."))]
    },
    "dependencies": ["cuda"]
}

ConfigFunc.export_config_json(__file__, config)
