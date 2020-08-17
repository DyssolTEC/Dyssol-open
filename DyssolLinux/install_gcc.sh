#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

# version to install
GCC_VER=9

# check current version
CURRENT_VERSION="$(gcc -dumpversion)"
if [ "$(printf '%s\n' "${GCC_VER}" "${CURRENT_VERSION}" | sort -V | head -n1)" = "${GCC_VER}" ]; then 
	echo "gcc " ${CURRENT_VERSION} " already installed."
	exit 1
fi

# add repository
apt -y install software-properties-common
add-apt-repository -y ppa:ubuntu-toolchain-r/test

# install gcc
apt -y install gcc-9
apt -y install g++-9

# set installed version as default. 'sudo update-alternatives --config gcc' to choose another version
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${CURRENT_VERSION} 50 --slave /usr/bin/g++ g++ /usr/bin/g++-${CURRENT_VERSION} --slave /usr/bin/gcov gcov /usr/bin/gcov-${CURRENT_VERSION}
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9                  70 --slave /usr/bin/g++ g++ /usr/bin/g++-9                  --slave /usr/bin/gcov gcov /usr/bin/gcov-9

# install additional tools
apt -y install build-essential

# test version
gcc --version
g++ --version
