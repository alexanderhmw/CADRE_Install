from MakeIt import ConfigFunc

name = "cuda"
version_list = [11]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'url': 'https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin',
        'file': "cuda-ubuntu2004.pin",
        'postcmds': ['sudo mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600',
                     'sudo apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/7fa2af80.pub',
                     'sudo add-apt-repository "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/ /"',
                     'sudo apt-get update',
                     'sudo apt-get install cuda-11-1 cuda-11-2 cuda-11-4 -y']
    },
    "ldpath": ['/usr/local/cuda'],
    "postcmds": {
        'cmd': 'echo "export CUDA_TOOLKIT_PATH=/usr/local/cuda" >> ~/.bashrc',
        'condition': '! grep -q "export CUDA_TOOLKIT_PATH=/usr/local/cuda" ~/.bashrc'
    }
}

ConfigFunc.export_config_json(__file__, config)
