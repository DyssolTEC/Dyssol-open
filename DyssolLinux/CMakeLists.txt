# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

cmake_minimum_required(VERSION 3.0)

project(DYSSOL VERSION 0.10.0.0 LANGUAGES CXX)

set(HDF5_INSTALL_PATH ./hdf5_install)
set(ZLIB_INSTALL_PATH ./zlib_install)
set(SUNDIALS_INSTALL_PATH ./sundials_install)

set(SRC_CORE "./src/Core")
set(SRC_UNITS "./src/Units")
set(SRC_SOLVERS "./src/Solvers")

set(UNITS_NAMES 
	"Agglomerator"
	"Bunker"
	"Crusher"
	"CrusherPBMTM"
	"Cyclone2"
	"Granulator"
	"HeatExchanger"
	"Inlet"
	"Mixer"
	"Mixer3"
	"Outlet"
	"Screen"
	"Splitter"
	"Splitter3"
	"TimeDelay"
)

set(SOLVERS_NAMES 
	"AgglomerationCellAverage"
	"AgglomerationFixedPivot"
	"AgglomerationFFT"
)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++17 -pthread")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

file(GLOB_RECURSE PROJ_SRC
	${SRC_CORE}/*.cpp
	${SRC_CORE}/*.c
	${SRC_CORE}/*.h
)

file(GLOB C_FILES ${SRC_CORE}/*.c)
set_source_files_properties(${C_FILES} PROPERTIES LANGUAGE CXX)

include_directories(
	${SRC_CORE} 
	${CMAKE_CURRENT_BINARY_DIR}
	${SRC_CORE}/include/
	${HDF5_INSTALL_PATH}/include/
	${SUNDIALS_INSTALL_PATH}/include/
) 

link_directories(${HDF5_INSTALL_PATH}/lib/ ${ZLIB_INSTALL_PATH}/lib/ ${SUNDIALS_INSTALL_PATH}/lib/)

add_executable(dyssol ${PROJ_SRC})

target_link_libraries(dyssol
	"hdf5_cpp"
	"hdf5_hl"
	"hdf5_hl_cpp"
	"hdf5"
	"z"
	"sundials_ida"
	"sundials_kinsol"
)

foreach(UNIT_NAME ${UNITS_NAMES})
	file(GLOB_RECURSE SRC_UNIT
		${SRC_UNITS}/${UNIT_NAME}/*.cpp
		${SRC_UNITS}/${UNIT_NAME}/*.h
	)
	add_library(unit_${UNIT_NAME} SHARED ${SRC_UNIT})
endforeach(UNIT_NAME)

foreach(SOLVER_NAME ${SOLVERS_NAMES})
	file(GLOB_RECURSE SRC_SOLVER
		${SRC_SOLVERS}/${SOLVER_NAME}/*.cpp
		${SRC_SOLVERS}/${SOLVER_NAME}/*.h
	)
	add_library(solver_${SOLVER_NAME} SHARED ${SRC_SOLVER})
endforeach(SOLVER_NAME)