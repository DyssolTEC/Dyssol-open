#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

# version to install
CMAKE_VER=3.18.0

# check current version
CURRENT_VERSION="$(cmake --version | head -n1 | cut -d' ' -f3)"
if [ "$(printf '%s\n' "${CMAKE_VER}" "${CURRENT_VERSION}" | sort -V | head -n1)" = "${CMAKE_VER}" ]; then 
	echo "cmake " ${CURRENT_VERSION} " already installed."
	exit 1
fi

# uninstall previous version
apt -y autoremove --purge cmake
apt -y autoremove --purge cmake-data

# install required packages
apt-get update
apt -y install apt-transport-https ca-certificates gnupg software-properties-common wget

# obtain a copy of signing key:
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null

# add repo
# sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ xenial main' # Ubuntu 16.04
apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' # Ubuntu 18.04
# sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal  main' # Ubuntu 20.04
apt-get update

# install additional package
apt -y install kitware-archive-keyring
rm /etc/apt/trusted.gpg.d/kitware.gpg

# install cmake
apt -y install cmake-data=${CMAKE_VER}-0kitware1
apt -y install cmake=${CMAKE_VER}-0kitware1

# test version
cmake --version
