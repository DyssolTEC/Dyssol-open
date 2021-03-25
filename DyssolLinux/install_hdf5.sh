#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

# path to install
PATH_TO_INSTALL=$(pwd)
ZLIB_INSTALL_PATH=$PATH_TO_INSTALL/external_libs/zlib
HDF5_INSTALL_PATH=$PATH_TO_INSTALL/external_libs/hdf5

# versions to install
ZLIB=zlib-1.2.11
HDF5_MAJOR=1
HDF5_MINOR=12
HDF5_BUILD=0

HDF5=hdf5-$HDF5_MAJOR.$HDF5_MINOR.$HDF5_BUILD

# clear old
rm -rf $ZLIB_INSTALL_PATH
rm -rf $HDF5_INSTALL_PATH

# build and install zlib
wget http://www.zlib.net/$ZLIB.tar.gz
tar -xvzf $ZLIB.tar.gz
cd $ZLIB
CC=gcc ./configure --prefix=$ZLIB_INSTALL_PATH
make -j 8
make install

# clean up zlib
cd ../
rm $ZLIB.tar.gz
rm -rf $ZLIB

# build and install hdf5
wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-$HDF5_MAJOR.$HDF5_MINOR/$HDF5/src/$HDF5.tar.gz
tar -xvzf $HDF5.tar.gz
cd $HDF5
CC=gcc ./configure --prefix=$HDF5_INSTALL_PATH --enable-cxx --enable-hl --disable-shared --enable-build-mode=production --with-zlib=$ZLIB_INSTALL_PATH/include,$ZLIB_INSTALL_PATH/lib
make -j 8
make install

# clean up hdf5
cd ../
rm $HDF5.tar.gz
rm -rf $HDF5
