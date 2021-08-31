import os
import sys
import subprocess
import getpass
import re
import tarfile
from zipfile import ZipFile
import json
from colorama import Fore, Back, Style
import wget
import pathlib


sudo_passwd = None
apt_updated = False


def inputSudoPasswd(init_passwd=False, gui_flag=False):
    global sudo_passwd
    if init_passwd:
        if gui_flag:
            sudo_passwd = subprocess.check_output(['zenity', '--password', '--title="sudo"'])
        else:
            sudo_passwd = getpass.getpass()
    elif sudo_passwd is None or sudo_passwd == "":
        sudo_passwd = getpass.getpass()
    return


inputSudoPasswd(True)


current_name = None

makeit_status = {}
success_log = {}
error_log = {}


makeit_path = pathlib.Path(__file__).parent.resolve()
makeit_status_filename = os.path.join(makeit_path, "makeit_status.json")
makeit_success_filename = os.path.join(makeit_path, "makeit_success.json")
makeit_error_filename = os.path.join(makeit_path, "makeit_error.json")

if os.path.isfile(makeit_status_filename):
    with open(makeit_status_filename, 'r') as fp:
        makeit_status = json.load(fp)
if os.path.isfile(makeit_success_filename):
    with open(makeit_success_filename, 'r') as fp:
        success_log = json.load(fp)
if os.path.isfile(makeit_error_filename):
    with open(makeit_error_filename, 'r') as fp:
        error_log = json.load(fp)


def runOsCmd(cmd, input=None, condition='true'):
    global current_name
    global success_log
    global error_log

    if type(cmd) == list:
        cmd = ' '.join(cmd)

    if input is None or len(input) == 0:
        print(Fore.GREEN + cmd + Fore.WHITE)
    else:
        print(Fore.GREEN + 'echo ' + str(input) + ' | ' + cmd + Fore.WHITE)

    if re.search(r'\b' + re.escape('echo') + r'\b', cmd):
        cmd = re.sub(r'\becho\b', '/bin/echo', cmd).strip()

    if type(input) == list:
        input = '; /bin/echo '.join(input)

    if re.search(r'\b' + re.escape('sudo') + r'\b', cmd):
        global sudo_passwd
        inputSudoPasswd()
        cmd = re.sub(r'\bsudo\b', '', cmd).strip()
        if input == None or len(input) == 0:
            status = subprocess.run(['sudo', '-Sk', '/bin/bash', '-is'],
                                    input='{}\nif {}; then {}; else exit 0; fi'.format(sudo_passwd, condition,
                                                                                       cmd).encode()).returncode
        else:
            status = subprocess.run(['sudo', '-Sk', '/bin/bash', '-is'],
                                    input='{}\nif {}; then ( /bin/echo {} ) | {} ; else exit 0; fi'.format(sudo_passwd,
                                                                                                           condition,
                                                                                                           input,
                                                                                                           cmd).encode()).returncode
    else:
        if input == None or len(input) == 0:
            status = subprocess.run(['/bin/bash', '-is'], input='if {} ; then {} ; else exit 0 ; fi'.format(condition,
                                                                                                            cmd).encode()).returncode
        else:
            status = subprocess.run(['/bin/bash', '-is'],
                                    input='if {}; then ( /bin/echo {} ) | {} ; else exit 0; fi'.format(condition, input,
                                                                                                       cmd).encode()).returncode

    if status != 0:
        error_log[current_name].append(str(cmd) + ' : ' + str(status))
        return False
    else:
        success_log[current_name].append(str(cmd) + ' : ' + str(status))
        return True


def runOsCmds(cmds):
    if cmds is not None:
        if type(cmds) == list:
            for cmd in cmds:
                if type(cmd) == str:
                    if not runOsCmd(cmd):
                        return False
                elif type(cmd) == dict:
                    if 'cmd' in cmd.keys():
                        if not runOsCmd(cmd.get('cmd'), cmd.get('input'), cmd.get('condition', 'true')):
                            return False
        elif type(cmds) == str:
            return runOsCmd(cmds)
        elif type(cmds) == dict:
            if 'cmd' in cmds.keys():
                return runOsCmd(cmds.get('cmd'), cmds.get('input'), cmds.get('condition', 'true'))
    return True


def aptInstall(sources=[], ppas=[], pkgs=[], remove=[], pips=[]):
    global apt_updated
    if sources is not None:
        for source in sources:
            runOsCmds({'cmd': 'sudo echo \'{}\' >> /etc/apt/sources.list'.format(source),
                       'condition': '! grep -q \'{}\' /etc/apt/sources.list'.format(source)})
        if len(sources) > 0:
            runOsCmd('sudo apt-get update')
            apt_updated = True

    if ppas is not None:
        for ppa in ppas:
            runOsCmd('sudo add-apt-repository -y {}'.format(ppa))
        if len(ppas) > 0:
            runOsCmd('sudo apt-get update')
            apt_updated = True

    if pkgs is not None and len(pkgs) > 0:
        inputSudoPasswd()
        if not apt_updated:
            runOsCmd('sudo apt-get update')
            apt_updated = True
        if not runOsCmd('sudo apt-get install -y {}'.format(' '.join(pkgs))):
            return False

    if remove is not None and len(remove) > 0:
        inputSudoPasswd()
        if not apt_updated:
            runOsCmd('sudo apt-get update')
            apt_updated = True
        if not runOsCmd('sudo apt-get remove -y {}'.format(' '.join(remove))):
            return False

    if pips is not None and len(pips) > 0:
        inputSudoPasswd()
        if not runOsCmd('python3 -m pip install {}'.format(' '.join(pips)), None, '[ -f /usr/local/bin/pip ]'):
            return False

    return True


def gitSource(root, name, url, branch, patch, precmds, postcmds):
    source_path = os.path.abspath(os.path.join(root, name))
    print(Fore.BLUE + source_path + Fore.WHITE)

    if not runOsCmds(precmds):
        return False

    os.chdir(root)
    is_in_repo = runOsCmd('git rev-parse --is-inside-work-tree')
    if not os.path.exists(source_path):
        if url is None:
            return False
        else:
            if is_in_repo:
                runOsCmd('git submodule add {} {}'.format(url, name))
            else:
                runOsCmd('git clone {} {}'.format(url, name))

    os.chdir(source_path)

    if len(os.listdir(source_path)) == 0 and is_in_repo:
        runOsCmd('git submodule init')
        runOsCmd('git submodule update -f')
    if branch is not None:
        if not runOsCmd('git checkout {}'.format(branch)):
            if not runOsCmd('git stash'):
                return False
            else:
                if not runOsCmd('git checkout {}'.format(branch)):
                    return False

    if patch is not None and type(patch) == list:
        for patch_file in patch:
            if os.path.exists(patch_file):
                os.chdir(root)
                if not runOsCmd('patch -p0 -N < {}'.format(patch_file), None,
                                '! patch -R -p0 -s -f --dry-run < {}'.format(patch_file)):
                    return False

    if not runOsCmds(postcmds):
        return False

    return os.path.exists(source_path)


def pkgSource(root, name, url, file, type_name, patch, precmds, postcmds):
    source_path = os.path.abspath(os.path.join(root, name))
    print(Fore.BLUE + source_path + Fore.WHITE)

    if not runOsCmds(precmds):
        return False

    is_in_repo = runOsCmd('git rev-parse --is-inside-work-tree')
    if file is None:
        pkg_path = os.path.abspath(os.path.join(root, name + '.' + type_name))
    else:
        pkg_path = os.path.abspath(os.path.join(root, file))

    print(Fore.BLUE + pkg_path + Fore.WHITE)
    if not os.path.exists(pkg_path):
        if url is None:
            return False
        else:
            wget.download(url, pkg_path)

    if ['tar', 'gz', 'tar.gz', 'bz2', 'xz', 'zip'].count(type_name) > 0:
        if not os.path.exists(source_path) and type_name is not None:
            if os.path.exists(pkg_path):
                if type_name == 'tar':
                    tar = tarfile.open(pkg_path, 'r')
                    tar.extractall(path=source_path)
                    tar.close()
                elif type_name == 'gz' or type_name == 'tar.gz':
                    tar = tarfile.open(pkg_path, 'r:gz')
                    tar.extractall(path=source_path)
                    tar.close()
                elif type_name == 'bz2':
                    tar = tarfile.open(pkg_path, 'r:bz2')
                    tar.extractall(path=source_path)
                    tar.close()
                elif type_name == 'xz':
                    tar = tarfile.open(pkg_path, 'r:xz')
                    tar.extractall(path=source_path)
                    tar.close()
                elif type_name == 'zip':
                    zip = ZipFile(pkg_path, 'r')
                    zip.extractall(source_path)
                    zip.close()

        if os.path.exists(source_path):
            if patch is not None and type(patch) == list:
                for patch_file in patch:
                    if os.path.exists(patch_file):
                        os.chdir(root)
                        if not runOsCmd('patch -p0 -N < {}'.format(patch_file), None,
                                        '! patch -R -p0 -s -f --dry-run < {}'.format(patch_file)):
                            return False

            os.chdir(source_path)
            if not runOsCmds(postcmds):
                return False

        return os.path.exists(source_path)

    elif ['deb', 'sh'].count(type_name) > 0:
        if os.path.exists(pkg_path):
            if type_name == 'deb':
                if not runOsCmd('sudo dpkg -i {}'.format(pkg_path)):
                    return False

            os.chdir(root)
            if not runOsCmds(postcmds):
                return False

        return os.path.exists(pkg_path)

    else:
        os.chdir(root)
        if not runOsCmds(postcmds):
            return False
        return True


def makePackage(root, name, param):
    if param.get('type', '') == 'cmake':
        build_path = os.path.abspath(os.path.join(root, name, param.get('src', '.'), 'build'))
        print(Fore.BLUE + build_path)
        if not os.path.exists(build_path):
            os.makedirs(build_path)
        os.chdir(build_path)
        if not runOsCmd('cmake {}'.format(' '.join(param.get('args', [])))):
            print(Fore.RED + name + ' cmake error' + Fore.WHITE)
            return False

    elif param.get('type', '') == 'qmake':
        build_path = os.path.abspath(os.path.join(root, name, param.get('src', '.')))
        print(Fore.BLUE + build_path)
        if not os.path.exists(build_path):
            print(Fore.RED + name + ' {} does not exist'.format(build_path) + Fore.WHITE)
            return False
        os.chdir(build_path)
        if not runOsCmd('qmake {}'.format(' '.join(param.get('args', [])))):
            print(Fore.RED + name + ' qmake error' + Fore.WHITE)
            return False

    elif param.get('type', '') == 'script':
        build_path = os.path.abspath(os.path.join(root, name, param.get('src', '.')))
        print(Fore.BLUE + build_path)
        if not os.path.exists(build_path):
            print(Fore.RED + name + ' {} does not exist'.format(build_path) + Fore.WHITE)
            return False
        os.chdir(build_path)
        if not runOsCmds(param.get('args')):
            print(Fore.RED + name + ' script error' + Fore.WHITE)
            return False
    else:
        build_path = os.path.abspath(os.path.join(root, name, param.get('src', '.')))
        print(Fore.BLUE + build_path)
        if not os.path.exists(build_path):
            print(Fore.RED + name + ' {} does not exist'.format(build_path) + Fore.WHITE)
            return False
        os.chdir(build_path)

    if param.get('make', []) is not None:
        if not runOsCmd('make {}'.format(' '.join(param.get('make', [])))):
            print(Fore.RED + name + ' make error' + Fore.WHITE)
            return False

    if param.get('install', []) is not None:
        if not runOsCmd('sudo make install {}'.format(' '.join(param.get('install', [])))):
            print(Fore.RED + name + ' make install error' + Fore.WHITE)
            return False

    return True


def makeit(name, version, apt=None, git=None, pkg=None, params=None, ldpath=None, alternatives=None, precmds=None, postcmds=None, dependencies=None,
           root=os.getcwd()):
    if name is None:
        return False

    if version is None:
        full_name = name
    else:
        full_name = "{}_{}".format(name, version)

    global sudo_passwd
    inputSudoPasswd()

    print(Fore.GREEN + full_name + ' start building' + Fore.WHITE)
    print(Fore.BLUE + root + Fore.WHITE)

    global current_name
    global makeit_status
    global success_log
    global error_log

    if name in makeit_status and makeit_status[name]:
        return True

    current_name = name;
    makeit_status[name] = False;
    success_log[name] = [];
    error_log[name] = [];

    if dependencies is not None:
        for dependency in dependencies:
            if not makeit_status.get(dependency, False):
                error_log[current_name].append('{}\'s dependency {} is not made'.format(full_name, dependency))
                print(Fore.RED + '{}\'s dependency {} is not made'.format(full_name, dependency) + Fore.WHITE)
                makeit_status[name] = False
                return False

    if apt is not None and not aptInstall(apt.get('sources'), apt.get('ppas'), apt.get('pkgs'), apt.get('remove'), apt.get('pips')):
        os.chdir(root)
        print(Fore.RED + full_name + ' aptInstall error' + Fore.WHITE)
        makeit_status[name] = False
        return False

    if not runOsCmds(precmds):
        os.chdir(root)
        print(Fore.RED + full_name + ' pre-processing error' + Fore.WHITE)
        makeit_status[name] = False
        return False

    print(root)
    git_path = os.path.join(root, "gits")
    print(git_path)
    os.makedirs(git_path, exist_ok=True)

    pkg_path = os.path.join(root, "pkgs")
    print(pkg_path)
    os.makedirs(pkg_path, exist_ok=True)



    if git is not None and git.get('url') is not None:
        if not gitSource(git_path, full_name, git.get('url'), git.get('branch'), git.get('patch'), git.get('precmds'), git.get('postcmds')):
            os.chdir(root)
            print(Fore.RED + full_name + ' gitSource error' + Fore.WHITE)
            makeit_status[name] = False
            return False
        if params is not None:
            if type(params) == dict:
                if not makePackage(git_path, full_name, params):
                    os.chdir(root)
                    makeit_status[name] = False
                    return False
            elif type(params) == list:
                for param in params:
                    if not makePackage(git_path, full_name, param):
                        os.chdir(root)
                        makeit_status[name] = False
                        return False
    elif pkg is not None and pkg.get('url') is not None or pkg.get('file') is not None:
        if not pkgSource(pkg_path, full_name, pkg.get('url'), pkg.get('file'), pkg.get('type'), pkg.get('patch'), pkg.get('precmds'), pkg.get('postcmds')):
            os.chdir(root)
            print(Fore.RED + full_name + ' pkgSource error' + Fore.WHITE)
            makeit_status[name] = False
            return False
        if params is not None:
            if type(params) == dict:
                if not makePackage(pkg_path, full_name, params):
                    os.chdir(root)
                    makeit_status[name] = False
                    return False
            elif type(params) == list:
                for param in params:
                    if not makePackage(pkg_path, full_name, param):
                        os.chdir(root)
                        makeit_status[name] = False
                        return False
    else:
        os.chdir(root)
        print(Fore.RED + full_name + 'Neither git nor pkg is defined' + Fore.WHITE)
        makeit_status[name] = False
        return False

    if ldpath is not None and len(ldpath) > 0:
        runOsCmd('sudo echo {} > /etc/ld.so.conf.d/{}.conf'.format(':'.join(ldpath), full_name))
        runOsCmd('sudo ldconfig')

    if alternatives is not None:
        for alternative in alternatives:
            alter_link = alternative.get('link', None)
            alter_name = alternative.get('name', None)
            alter_path = alternative.get('path', None)
            alter_priority = alternative.get('priority', None)
            alter_slave_link = alternative.get('slave_link', None)
            alter_slave_name = alternative.get('slave_name', None)
            alter_slave_path = alternative.get('slave_path', None)
            if alter_link is not None and alter_name is not None and alter_path is not None and alter_priority is not None:
                if alter_slave_link is not None and alter_slave_name is not None and alter_slave_path is not None:
                    runOsCmd(
                        ['sudo', 'update-alternatives', '--install', alter_link, alter_name, alter_path, alter_priority,
                         "--slave", alter_slave_link, alter_slave_name, alter_slave_path])
                else:
                    runOsCmd(['sudo', 'update-alternatives', '--install', alter_link, alter_name, alter_path,
                              alter_priority])

    if not runOsCmds(postcmds):
        os.chdir(root)
        print(Fore.RED + full_name + ' post-processing error' + Fore.WHITE)
        makeit_status[name] = False
        return False

    os.chdir(root)
    print(Fore.GREEN + full_name + ' is done' + Fore.WHITE)
    makeit_status[name] = True
    return True


def saveMakeLog():
    global makeit_status
    global success_log
    global error_log

    with open(makeit_status_filename, 'w') as fp:
        json.dump(makeit_status, fp, indent=4, sort_keys=True)
    with open(makeit_success_filename, 'w') as fp:
        json.dump(success_log, fp, indent=4, sort_keys=True)
    with open(makeit_error_filename, 'w') as fp:
        json.dump(error_log, fp, indent=4, sort_keys=True)


def printMakeLog(details=False):
    global makeit_status
    global success_log
    global error_log

    for name in makeit_status:
        if makeit_status[name]:
            print(Fore.GREEN + '{} succeeded'.format(name) + Fore.WHITE)
            if details:
                for log in success_log[name]:
                    print(Fore.WHITE + log)
        else:
            print(Fore.RED + '{} failed'.format(name) + Fore.WHITE)
            if details:
                for log in error_log[name]:
                    print(Fore.WHITE + log)


def getQTDIR():
    return '/' + '/'.join(str(subprocess.check_output(['qmake', '--version'], shell=False)).split('/')[1:-1])


print('QTDIR=' + getQTDIR())


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