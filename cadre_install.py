import argparse
import json
import os
import runpy
from MakeIt import DeployFunc
import pathlib

if __name__ == "__main__":
    # parser = argparse.ArgumentParser(description="Install CADRE")
    # parser.add_argument("cadre_path", type=str, help="path to tartanracing")
    # args = parser.parse_args()

    root_path = os.path.abspath(pathlib.Path(__file__).parent.resolve())
    configs_path = os.path.abspath(os.path.join(root_path, "./configs"))

    # collect arcgis.py files and remove old config json files
    makeit_list = []
    for (dirpath, dirnames, filenames) in os.walk(configs_path):
        for filename in filenames:
            if filename.endswith(".py"):
                makeit_list.append(os.path.abspath(os.path.join(dirpath, filename)))
            elif filename.endswith(".json"):
                os.remove(os.path.join(dirpath, filename))

    # run arcgis.py files to generate new config json files
    for makeit_file in makeit_list:
        print(makeit_file)
        runpy.run_path(makeit_file)

    # collect with new config json files
    config_json_queue = []
    for (dirpath, dirnames, filenames) in os.walk(configs_path):
        for filename in filenames:
            if filename.endswith(".json"):
                config_json_queue.append(os.path.abspath(os.path.join(dirpath, filename)))

    # deploy
    config_json_queue_size = len(config_json_queue)
    config_json_failed_queue_size = 0
    config_json_failed_queue = []
    try:
        while config_json_queue_size > config_json_failed_queue_size:
            config_json_queue_size = len(config_json_queue)
            for config_json in config_json_queue:
                with open(config_json, "r") as f:
                    config = json.load(f)
                    make_status = DeployFunc.makeit(name=config.get("name"),
                                                    version=config.get("version"),
                                                    apt=config.get("apt"),
                                                    git=config.get("git"),
                                                    pkg=config.get("pkg"),
                                                    params=config.get("params"),
                                                    ldpath=config.get("ldpath"),
                                                    alternatives=config.get("alternatives"),
                                                    precmds=config.get("precmds"),
                                                    postcmds=config.get("postcmds"),
                                                    dependencies=config.get("dependencies"),
                                                    root=root_path)
                    DeployFunc.saveMakeLog()
                    if not make_status:
                        config_json_failed_queue.append(config_json)

            config_json_failed_queue_size = len(config_json_failed_queue)
            print(config_json_queue_size, config_json_failed_queue_size)
            print(config_json_queue, config_json_failed_queue)

            config_json_queue = config_json_failed_queue.copy()
            config_json_failed_queue.clear()

    except KeyboardInterrupt:
        pass
    DeployFunc.printMakeLog(True)
