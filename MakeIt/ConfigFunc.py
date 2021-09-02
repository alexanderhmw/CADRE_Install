import json
import os
import pathlib
import subprocess


def version_str(version_list, sep, version_idx_num=0):
    version_str_list = [str(version) for version in version_list]
    if version_idx_num <= 0 or version_idx_num > len(version_str_list):
        return sep.join(version_str_list)
    else:
        return sep.join(version_str_list[:version_idx_num])


def name_version_str(name, version_list):
    return "{}_{}".format(name, version_str(version_list, "_"))


def resolve_file_path(file):
    return pathlib.Path(file).parent.resolve()


def export_config_json(file, config):
    filename = os.path.join(resolve_file_path(file), "{}_{}.json".format(config["name"], config["version"]))
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    with open(filename, "w") as f:
        json.dump(config, f, sort_keys=True, indent=2)


def to_absolute_path(files, dir="./"):
    if type(files) == list:
        return [os.path.abspath(os.path.join(dir, file)) for file in files]
    elif type(files) == str:
        return os.path.abspath(os.path.join(dir, files))


def filterPATH(PATH):
    sysPATH=['/usr/local/lib','/usr/lib','/usr/lib/x86_64-linux-gnu',
             '/usr/local/include', '/usr/include']
    PATH = [l.strip().rstrip() for l in PATH]
    PATH = [l for l in PATH if not l in sysPATH]
    return PATH


def getQTDIR():
    return '/' + '/'.join(str(subprocess.check_output(['qmake', '--version'], shell=False)).split('/')[1:-1])


def getQTCPPPATH(QtFilter=None):
    QTDIR = getQTDIR()
    QTCPPPATH = []
    QTCPPPATH.append(os.path.join(QTDIR, "include"))
    ret = os.listdir(os.path.join(QTDIR, "include"))
    if QtFilter is not None:
        ret = [r for r in ret if r in QtFilter]
    ret = [os.path.join(QTDIR, "include", r) for r in ret]
    ret = [r for r in ret if os.path.isdir(r)]
    QTCPPPATH.extend(ret)
    return filterPATH(QTCPPPATH)


def getQTLIBPATH():
    QTDIR = getQTDIR()
    return filterPATH([os.path.join(QTDIR, "lib")])


def getQTLIBS(QtFilter=None):
    QTLIBPATH = getQTLIBPATH()[0]
    ret = os.listdir(QTLIBPATH)
    ret = [r for r in ret if not os.path.isdir(os.path.join(QTLIBPATH, r))]
    ret = [r for r in ret if r.endswith(".so") and r.startswith("libQt5")]
    ret = [r[3:-3] for r in ret]
    if QtFilter is not None:
        ret = [r for r in ret if r in QtFilter]
    return ret