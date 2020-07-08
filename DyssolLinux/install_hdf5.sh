#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

PATH_TO_INSTALL=$(pwd)
ZLIB_INSTALL_PATH=$PATH_TO_INSTALL/zlib_install
HDF5_INSTALL_PATH=$PATH_TO_INSTALL/hdf5_install

ZLIB=zlib-1.2.11
HDF5_MAJOR=1
HDF5_MINOR=10
HDF5_BUILD=2

HDF5=hdf5-$HDF5_MAJOR.$HDF5_MINOR.$HDF5_BUILD

# Clear old
rm -rf $ZLIB_INSTALL_PATH
rm -rf $HDF5_INSTALL_PATH

# Build zlib
wget http://www.zlib.net/$ZLIB.tar.gz
tar -xvzf $ZLIB.tar.gz
cd $ZLIB
CC=gcc ./configure --prefix=$ZLIB_INSTALL_PATH
make -j 8
make install

cd ../
rm $ZLIB.tar.gz
rm -rf $ZLIB

# Build hdf5
wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-$HDF5_MAJOR.$HDF5_MINOR/$HDF5/src/$HDF5.tar.gz
tar -xvzf $HDF5.tar.gz
cd $HDF5
CC=gcc ./configure --prefix=$HDF5_INSTALL_PATH --enable-cxx --enable-hl --disable-shared --enable-build-mode=production --with-zlib=$ZLIB_INSTALL_PATH/include,$ZLIB_INSTALL_PATH/lib
make -j 8
make install

# Clean
cd ../
rm $HDF5.tar.gz
rm -rf $HDF5