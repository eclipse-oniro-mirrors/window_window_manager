#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Description
#
# This script is invoked by the build system and does not need to be executed directly by the developer.
# First, it checks if --release is provided as an argument. This is the only allowed type for stateMgmt that is included in the build image.
# It then verifies if the node_modules folder exists. If not, npm install is executed.
# Afterward, npm run build_release is performed, which also generates generateGni.js
# The files_to_watch.gni file contains a list of input files from tsconfig.base.json.
# When any of these files are modified, the build system triggers this script to regenerate stateMgmt.js.

import os
import sys
import time
import shutil
import subprocess
import re


def copy_all_js_files(source_dir, target_dir):
    if not os.path.exists(source_dir):
        print(f"windowenv: The source directory does not exist. {source_dir}")
        return False

    copied_files = 0
    for filename in os.listdir(source_dir):
        if filename.endswith('.js'):
            src_file = os.path.join(source_dir, filename)
            dst_file = os.path.join(target_dir, filename)
            try:
                shutil.copy(src_file, dst_file)
                copied_files += 1
            except Exception as e:
                print(f"windowenv: copy file failed {filename}: {str(e)}")

    if copied_files == 0:
        print(f"windowenv: {source_dir} not found js file in the directory")
        return False
    return True

def main(argv):
    if len(argv) < 4:
        print("windowenv: python build.py <path_to_project> <path_to_node_modules> <js_output_path>")
        sys.exit(1)

    project_path = os.path.abspath(argv[1])
    node_modules_path = os.path.abspath(argv[2])
    js_output_path = os.path.abspath(argv[3])

    print(f"windowenv: Changing directory to {project_path}. Out dir = {js_output_path}")
    if not os.path.exists(project_path):
        print(f"windowenv: {project_path} not exist")
        sys.exit(1)

    os.chdir(project_path)

    # Check if `node_modules` exists. If yes skip npm install
    if not os.path.exists(node_modules_path):
        print(f"windowenv: node_modules directory not found at {node_modules_path}, running npm install")
        install_dir = os.path.dirname(node_modules_path)
        try:
            subprocess.check_call(["npm", "install", "--prefix", install_dir])
        except subprocess.CalledProcessError as e:
            print(f"windowenv: npm install failed with exit code {e.returncode}. Retry...")
            print(e.stderr)
            secondary_npm_registry = "https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/"
            try:
                subprocess.check_call([
                    "npm", "install",
                    "--prefix", install_dir,
                    "--registry", secondary_npm_registry,
                    "--loglevel=verbose"
                ])
            except subprocess.CalledProcessError as e2:
                print(e2.stderr)
                sys.exit(e2.returncode)
    else:
        print(f"windowenv: node_modules directory exists at {node_modules_path}")

    # Determine the npm script to run. Currently only build_release supported.
    script = "build_release"
    print(f"windowenv: Running npm script '{script}'")

    try:
        subprocess.check_call(["npm", "run", script])
    except subprocess.CalledProcessError as e:
        print(f"windowenv: npm run {script} failed with exit code {e.returncode}.")
        sys.exit(e.returncode)

    # Ensure the output directory exists
    if not os.path.exists(js_output_path):
        os.makedirs(js_output_path)

    source_folder = "engine"
    engine_path = os.path.join(project_path, source_folder)
    if not copy_all_js_files(engine_path, js_output_path):
        print("windowenv: No available build output file found")
        sys.exit(1)

    # clean the tmp directory
    for dir_path in [engine_path, node_modules_path]:
        try:
            shutil.rmtree(dir_path)
            print(f"delete directory success: {dir_path}")
        except Exception as e:
            print(f"delete directory failed {dir_path}: {str(e)}")

if __name__ == '__main__':
    start_time = time.time()
    main(sys.argv)
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"windowenv: build time: {elapsed_time:.2f} seconds")
