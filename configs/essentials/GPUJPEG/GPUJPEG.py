from MakeIt import ConfigFunc

name = "GPUJPEG"
version_list = [1, 0, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://github.com/CESNET/GPUJPEG.git',
        'branch': 'master'
    },
    "params": {
        'src': '.',
        'type': 'cmake',
        'args': ['..'],
        'make': ['-j8']
    },
    "dependencies": ["OpenCV3", "cuda11"]
}

ConfigFunc.export_config_json(__file__, config)
