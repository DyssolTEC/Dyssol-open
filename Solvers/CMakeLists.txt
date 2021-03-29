# Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

set(SolversNames
    "AgglomerationCellAverage"
    "AgglomerationFFT"
    "AgglomerationFixedPivot"
)

foreach(foldername ${SolversNames})
    file(GLOB_RECURSE src ${CMAKE_SOURCE_DIR}/Solvers/${foldername}/*.cpp *.c)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Solvers/${foldername}/)
    add_library(${foldername} SHARED ${src})
    INSTALL(TARGETS ${foldername} DESTINATION ${INSTALL_UNITS_LIB_PATH})
endforeach(foldername ${SolversNames})
