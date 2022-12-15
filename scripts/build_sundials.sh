#!/bin/sh

SUNDIALS_WORK_PATH=~/Dyssol/
SUNDIALS_BUILD_PATH=${SUNDIALS_WORK_PATH}/build/
SUNDIALS_INSTALL_PATH=${SUNDIALS_WORK_PATH}/install/
SUNDIALS_MAJOR_VERSION=6
SUNDIALS_MIDDLE_VERSION=4
SUNDIALS_MINOR_VERSION=1
SUNDIALS_VERSION=${SUNDIALS_MAJOR_VERSION}.${SUNDIALS_MIDDLE_VERSION}.${SUNDIALS_MINOR_VERSION}

mkdir -p ${SUNDIALS_WORK_PATH}
mkdir -p ${SUNDIALS_BUILD_PATH}
mkdir -p ${SUNDIALS_INSTALL_PATH}
cd ${SUNDIALS_WORK_PATH}
wget https://github.com/LLNL/sundials/releases/download/v${SUNDIALS_VERSION}/sundials-${SUNDIALS_VERSION}.tar.gz
tar -xvf sundials-${SUNDIALS_VERSION}.tar.gz
cd sundials-${SUNDIALS_VERSION}/
cd ${SUNDIALS_BUILD_PATH}
cmake ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/ -DCMAKE_INSTALL_PREFIX:PATH=${SUNDIALS_INSTALL_PATH} -DBUILD_ARKODE=NO -DBUILD_CVODE=NO -DBUILD_CVODES=NO -DBUILD_IDA=YES -DBUILD_IDAS=NO -DBUILD_KINSOL=YES -DBUILD_SHARED_LIBS=NO -DBUILD_STATIC_LIBS=YES -DEXAMPLES_ENABLE_C=NO -DEXAMPLES_ENABLE_CXX=NO -DEXAMPLES_INSTALL=NO
cmake --build .
cmake --install .
if [ ${SUNDIALS_MAJOR_VERSION} -eq 5 ]
then
  cp ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/src/ida/ida_impl.h    ${SUNDIALS_INSTALL_PATH}/include/ida/
  cp ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/src/ida/ida_ls_impl.h ${SUNDIALS_INSTALL_PATH}/include/ida/
elif [ ${SUNDIALS_MAJOR_VERSION} -eq 6 ]
then
  cp ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/src/ida/ida_impl.h                   ${SUNDIALS_INSTALL_PATH}/include/ida/ida_impl.h
  cp ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/src/ida/ida_ls_impl.h                ${SUNDIALS_INSTALL_PATH}/include/ida/ida_ls_impl.h
  cp ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/src/sundials/sundials_context_impl.h ${SUNDIALS_INSTALL_PATH}/include/sundials_context_impl.h
  cp ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/src/sundials/sundials_hashmap.h      ${SUNDIALS_INSTALL_PATH}/include/sundials_hashmap.h
  cp ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}/src/sundials/sundials_logger_impl.h  ${SUNDIALS_INSTALL_PATH}/include/sundials_logger_impl.h
fi

rm ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}.tar.gz
rm -r ${SUNDIALS_WORK_PATH}/sundials-${SUNDIALS_VERSION}
rm -r ${SUNDIALS_BUILD_PATH}
