#!/bin/bash

set -euo pipefail

adb push build-linux-aarch64/bin/camlog /home/phablet/Workspace/camlog-builds/
adb push run_libertine.sh /home/phablet/Workspace/run_libertine.sh
adb shell cp -r /home/phablet/Workspace/camlog-builds/ /home/phablet/.cache/libertine-container/camlog-container/rootfs/root/
