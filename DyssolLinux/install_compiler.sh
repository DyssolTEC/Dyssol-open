#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

# gcc7
sudo apt update -y && \
sudo apt upgrade -y && \
sudo apt dist-upgrade -y && \
sudo apt install build-essential software-properties-common -y && \
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
sudo apt update -y && \
sudo apt install gcc-7 g++-7 -y && \
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 --slave /usr/bin/g++ g++ /usr/bin/g++-7 && \
sudo update-alternatives --config gcc

# cmake
sudo apt remove cmake
sudo apt purge --auto-remove cmake
version=3.11
build=2
wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz
tar -xzvf cmake-$version.$build.tar.gz
cd cmake-$version.$build/
./configure 
make -j 8
sudo make install
sudo update-alternatives --install /usr/bin/cmake cmake /usr/local/bin/cmake 1 --force
cd ../
rm cmake-$version.$build.tar.gz
rm -rf cmake-$version.$build

# test versions
gcc --version
g++ --version
cmake --version