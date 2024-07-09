#!/bin/bash

# Copyright (c) 2020, Dyssol Development Team. All rights reserved. 
# Copyright (c) 2023, DyssolTEC GmbH. All rights reserved. 
# This file is part of Dyssol. See LICENSE file for license information.

# Absolute path to this script, e.g. /home/user/bin/script.sh
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=$(dirname "$SCRIPT")
# Run copy
rsync -av --exclude=.git --exclude=.vs --exclude=build --exclude=Documentation/sources --exclude=DyssolConsole/cache --exclude=DyssolInstallers/Compiler --exclude=DyssolInstallers/Installers --exclude=ExternalLibraries --exclude=install --exclude=Win32 --exclude=x64 ${SCRIPTPATH}/../ ~/dyssol/