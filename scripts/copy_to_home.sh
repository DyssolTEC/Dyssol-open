#!/bin/bash

# Copyright (c) 2020, Dyssol Development Team. All rights reserved. 
# Copyright (c) 2023, DyssolTEC GmbH. All rights reserved. 
# This file is part of Dyssol. See LICENSE file for license information.

rsync -av --exclude=.git --exclude=.vs --exclude=build --exclude=DyssolInstallers/Compiler --exclude=DyssolInstallers/Installers --exclude=ExternalLibraries --exclude=install --exclude=Win32 --exclude=x64 /mnt/d/Codes/dyssol ~/