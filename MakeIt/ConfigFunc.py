import json
import os
import pathlib


def version_str(version_list, sep, version_idx_num=0):
    version_str_list = [str(version) for version in version_list]
    if version_idx_num <= 0 or version_idx_num > len(version_str_list):
        return sep.join(version_str_list)
    else:
        return sep.join(version_str_list[:version_idx_num])


def name_version_str(name, version_list):
    return "{}_{}".format(name, version_str(version_list, "_"))


def export_config_json(file, config):
    filename = os.path.join(pathlib.Path(file).parent.resolve(), "{}_{}.json".format(config["name"], config["version"]))
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    with open(filename, "w") as f:
        json.dump(config, f, sort_keys=True, indent=2)


def to_absolute_path(files):
    if type(files) == list:
        return [os.path.abspath(file) for file in files]
    elif type(files) == str:
        return os.path.abspath(files)
