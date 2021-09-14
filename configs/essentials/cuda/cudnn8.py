from MakeIt import ConfigFunc
from configs.essentials.cuda import cuda11

name = "cudnn8"
version_list = [8, 2, 2, 26]
subversion = 1

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'url': 'https://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu2004/x86_64/nvidia-machine-learning-repo-ubuntu2004_1.0.0-1_amd64.deb',
        'type': 'deb',
        'postcmds': ['sudo apt-get update',
                     'sudo apt-get install libcudnn8={}-{}+cuda{} -y'.format(ConfigFunc.version_str(version_list, "."), subversion, ConfigFunc.version_str(
                         cuda11.version_list, ".")),
                     'sudo apt-get install libcudnn8-dev={}-{}+cuda{} -y'.format(ConfigFunc.version_str(version_list, "."), subversion, ConfigFunc.version_str(
                         cuda11.version_list, "."))]
    },
    "dependencies": ["cuda11"]
}

ConfigFunc.export_config_json(__file__, config)
