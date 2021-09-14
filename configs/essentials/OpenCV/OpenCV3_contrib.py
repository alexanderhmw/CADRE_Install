from MakeIt import ConfigFunc

name = "OpenCV3_contrib"
version_list = [3, 4, 15]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "git": {
        'url': 'https://github.com/opencv/opencv_contrib.git',
        'branch': ConfigFunc.version_str(version_list, ".")
    }
}

ConfigFunc.export_config_json(__file__, config)
