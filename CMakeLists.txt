# Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.
# Copyright (c) 2021, Anton Gladky <Anton.Gladky@iart.tu-freiberg.de>, IART, Technical University Bergakademie Freiberg


cmake_minimum_required(VERSION 3.1.0)

project(Dyssol VERSION 0.10.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

INCLUDE(GNUInstallDirs)

SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


add_executable(DyssolC
    ${CMAKE_SOURCE_DIR}/DyssolConsole/main.cpp
)

set(libdyssolnames
	"BaseSolvers"
	"CacheHandler"
	"EquationSolvers"
	"HDF5Handler"
	"MaterialsDatabase"
	"ModelsAPI"
	"Modules"
	"SimulatorCore"
	"Utilities"
	"Modules/DustFormationTester"
)

foreach(foldername ${libdyssolnames})
    file(GLOB_RECURSE src   ${CMAKE_SOURCE_DIR}/${foldername}/*.cpp)
    set(libdyssolsrc ${libdyssolsrc} ${src})
    INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${foldername}/")
endforeach(foldername ${libdyssolnames})

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

FIND_PACKAGE(HDF5 REQUIRED COMPONENTS "CXX" "HL")
INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})

FIND_PACKAGE(SUNDIALS REQUIRED)
INCLUDE_DIRECTORIES(${SUNDIALS_INCLUDE_DIRS})

ADD_LIBRARY(libdyssol_shared SHARED ${libdyssolsrc})
ADD_LIBRARY(libdyssol_static STATIC ${libdyssolsrc})

TARGET_LINK_LIBRARIES(libdyssol_shared ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} ${SUNDIALS_LIBRARIES})
TARGET_LINK_LIBRARIES(libdyssol_static ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} ${SUNDIALS_LIBRARIES})
TARGET_LINK_LIBRARIES(DyssolC libdyssol_shared)

SET(INSTALL_UNITS_LIB_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/Dyssol/Units)
SET(INSTALL_MATERIALDB_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/Dyssol/MaterialsDB)
SET(INSTALL_DOCS_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/Dyssol/docs)

CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/DyssolMainWindow/config.ini.in" "${CMAKE_BINARY_DIR}/config.ini")
INSTALL(FILES "${CMAKE_BINARY_DIR}/config.ini" DESTINATION ${CMAKE_INSTALL_DATADIR})

INSTALL(FILES "${CMAKE_SOURCE_DIR}/Materials.dmdb" DESTINATION ${INSTALL_MATERIALDB_PATH})

INSTALL(TARGETS DyssolC DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(TARGETS libdyssol_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL(TARGETS libdyssol_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/Documentation/ DESTINATION ${INSTALL_DOCS_PATH})

ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/Units")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/DyssolMainWindow")