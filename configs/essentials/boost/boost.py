from MakeIt import ConfigFunc

name = "boost"
version_list = [1, 77, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        "url": "https://phoenixnap.dl.sourceforge.net/project/boost/boost/{}/boost_{}.tar.gz".format(
            ConfigFunc.version_str(version_list, "."),
            ConfigFunc.version_str(version_list, "_")),
        "type": "tar.gz",
    },
    "params": {
        "src": ConfigFunc.name_version_str(name, version_list),
        "type": "script",
        "args": ["./bootstrap.sh --prefix=/opt/boost/{}".format(ConfigFunc.version_str(version_list, "_")),
                 "./b2 --prefix=/opt/boost/{}".format(ConfigFunc.version_str(version_list, "_")),
                 "sudo ./b2 install --prefix=/opt/boost/{}".format(ConfigFunc.version_str(version_list, "_"))],
        "make": None,
        "install": None
    },
    "ldpath": ["/opt/boost/{}/lib".format(ConfigFunc.version_str(version_list, "_"))],
    "postcmds": ["sudo ln -sf /opt/boost/{}/include/ /opt/boost/include".format(ConfigFunc.version_str(version_list, "_")),
                 "sudo ln -sf /opt/boost/{}/lib/ /opt/boost/lib".format(ConfigFunc.version_str(version_list, "_"))]
}

ConfigFunc.export_config_json(__file__, config)
