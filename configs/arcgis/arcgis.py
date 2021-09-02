from MakeIt import ConfigFunc
from pathlib import Path

name = "arcgis"
version_list = [100, 12, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        "file": "ArcGIS_Runtime_SDK_Qt_Linux64_{}.tar.gz".format(ConfigFunc.version_str(version_list, "_")),
        "type": "gz"
    },
    "params": {
        "src": "ArcGIS_Runtime_SDK_Qt_Linux64",
        "type": "script",
        "args": [{"cmd": "./Setup -s -d ~/arcgis",
                  "condition": "[ ! -d ~/arcgis/runtime_sdk/qt{} ]".format(ConfigFunc.version_str(version_list, ".", 2))},
                 {"cmd": "~/arcgis/runtime_sdk/qt{}/postInstaller -s --qtcreator /opt/Qt/Tools/QtCreator/bin".format(ConfigFunc.version_str(version_list, ".", 2)),
                  "condition": "[ -f ~/arcgis/runtime_sdk/qt{}/postInstaller ]".format(ConfigFunc.version_str(version_list, ".", 2))},
                 "sudo ln -sf {}/arcgis/runtime_sdk/qt{}/sdk /opt/arcgis_sdk".format(Path.home(), ConfigFunc.version_str(version_list, ".", 2))],
        "make": None,
        "install": None
    },
    "ldpath": ["/opt/arcgis_sdk/linux/x64/lib"]
}

ConfigFunc.export_config_json(__file__, config)
