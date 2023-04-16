#!/bin/sh
# run 
# export SUNDIALS_ROOT=/path/to/custom/sundials/
# before running this script

DYSSOL_SRC_PATH=/mnt/d/Projects/Dyssol/
DYSSOL_BUILD_PATH=${DYSSOL_SRC_PATH}/build/ubuntu20/

mkdir -p ${DYSSOL_BUILD_PATH}
cd ${DYSSOL_BUILD_PATH}
cmake ${DYSSOL_SRC_PATH}
cmake --build . --parallel 8
make test
