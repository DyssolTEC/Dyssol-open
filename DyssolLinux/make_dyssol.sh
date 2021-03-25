#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

export CC=gcc
export CXX=g++
if [ ! -d build ]; then
	mkdir build
fi
cd build
cmake ../
make -j 8
cd ../

mkdir ./compiled
cp -v build/dyssol ./compiled/dyssol
chmod +x ./compiled/dyssol

cp -v build/*.so ./compiled

./compiled/dyssol -version
