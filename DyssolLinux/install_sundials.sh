#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

# path to install
PATH_TO_INSTALL=$(pwd)
SUNDIALS_INSTALL_PATH=$PATH_TO_INSTALL/external_libs/sundials

# version to install
SUNDIALS=sundials-5.3.0

# clear old
rm -rf $SUNDIALS_INSTALL_PATH

# build and install sundials
wget https://computation.llnl.gov/projects/sundials/download/$SUNDIALS.tar.gz
tar -xvzf $SUNDIALS.tar.gz
cd $SUNDIALS
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$SUNDIALS_INSTALL_PATH -DBUILD_ARKODE=NO -DBUILD_CVODE=NO -DBUILD_CVODES=NO -DBUILD_IDA=YES -DBUILD_IDAS=NO -DBUILD_KINSOL=YES -DBUILD_SHARED_LIBS=NO -DBUILD_STATIC_LIBS=YES -DEXAMPLES_ENABLE_C=NO -DEXAMPLES_ENABLE_CXX=NO -DEXAMPLES_INSTALL=NO ..
make -j 8
make install

# copy additional headers
cp ../src/ida/ida_impl.h $SUNDIALS_INSTALL_PATH/include/ida/ida_impl.h
cp ../src/ida/ida_ls_impl.h $SUNDIALS_INSTALL_PATH/include/ida/ida_ls_impl.h
cp ../src/kinsol/kinsol_impl.h $SUNDIALS_INSTALL_PATH/include/kinsol/kinsol_impl.h
cp ../src/kinsol/kinsol_ls_impl.h $SUNDIALS_INSTALL_PATH/include/kinsol/kinsol_ls_impl.h

# clean up
cd ../
rm -rf build
cd ../
rm $SUNDIALS.tar.gz
rm -rf $SUNDIALS
