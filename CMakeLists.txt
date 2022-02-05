# Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.
# Copyright (c) 2021, Anton Gladky <Anton.Gladky@iart.tu-freiberg.de>, IART, Technical University Bergakademie Freiberg


cmake_minimum_required(VERSION 3.1.0)

set (VERSION 1.0.0)
set (SOVERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Dyssol VERSION ${VERSION} LANGUAGES CXX)

INCLUDE(GNUInstallDirs)

SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

SET(INSTALL_UNITS_LIB_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/Dyssol/Units)
SET(INSTALL_MATERIALDB_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/Dyssol/MaterialsDB)
SET(INSTALL_DOCS_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/Dyssol/docs)
SET(INSTALL_CONFIG_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/Dyssol/)
SET(INSTALL_HEADERS_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/)

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
	"ScriptInterface"
	"SimulatorCore"
	"Utilities"
	"Modules/DustFormationTester"
)

foreach(foldername ${libdyssolnames})
    file(GLOB_RECURSE src   ${CMAKE_SOURCE_DIR}/${foldername}/*.cpp)
    file(GLOB_RECURSE hdr   ${CMAKE_SOURCE_DIR}/${foldername}/*.h)
    set(libdyssolsrc ${libdyssolsrc} ${src})
    INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/${foldername}/")
    INSTALL(FILES ${hdr} DESTINATION ${INSTALL_HEADERS_PATH}/Dyssol/${foldername}/)
endforeach(foldername ${libdyssolnames})

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

FIND_PACKAGE(HDF5 REQUIRED COMPONENTS "CXX" "HL")
INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})

FIND_PACKAGE(SUNDIALS REQUIRED COMPONENTS sundials_ida sundials_kinsol sundials_nvecserial)
INCLUDE_DIRECTORIES(${SUNDIALS_INCLUDE_DIRS})

ADD_LIBRARY(libdyssol_shared SHARED ${libdyssolsrc})
SET_TARGET_PROPERTIES(libdyssol_shared PROPERTIES
  VERSION ${VERSION}
  SOVERSION ${SOVERSION}
  PREFIX ""
  OUTPUT_NAME "libdyssol"
)

ADD_LIBRARY(libdyssol_static STATIC ${libdyssolsrc})
SET_TARGET_PROPERTIES(libdyssol_static PROPERTIES
  VERSION ${VERSION}
  SOVERSION ${SOVERSION}
  PREFIX ""
  OUTPUT_NAME "libdyssol"
)

target_compile_definitions(libdyssol_shared PRIVATE INSTALL_DOCS_PATH="${INSTALL_DOCS_PATH}")
target_compile_definitions(libdyssol_static PRIVATE INSTALL_DOCS_PATH="${INSTALL_DOCS_PATH}")

TARGET_LINK_LIBRARIES(libdyssol_shared ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} ${SUNDIALS_LIBRARIES})
TARGET_LINK_LIBRARIES(libdyssol_static ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} ${SUNDIALS_LIBRARIES})
TARGET_LINK_LIBRARIES(DyssolC libdyssol_shared)

FIND_PACKAGE(Graphviz)
IF(Graphviz_FOUND)
  MESSAGE(STATUS "Graphviz is found")
  MESSAGE(STATUS ${Graphviz_INCLUDE_DIR})
  ADD_DEFINITIONS("-DGRAPHVIZ")

  TARGET_LINK_LIBRARIES(libdyssol_shared ${Graphviz_LIBRARIES})
  TARGET_LINK_LIBRARIES(libdyssol_static ${Graphviz_LIBRARIES})

  INCLUDE_DIRECTORIES(${Graphviz_INCLUDE_DIR})
ENDIF(Graphviz_FOUND)

CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/DyssolMainWindow/config.ini.in" "${CMAKE_BINARY_DIR}/config.ini")
INSTALL(FILES "${CMAKE_BINARY_DIR}/config.ini" DESTINATION ${INSTALL_CONFIG_PATH})
INSTALL(FILES "${CMAKE_SOURCE_DIR}/Materials.dmdb" DESTINATION ${INSTALL_MATERIALDB_PATH})

INSTALL(TARGETS DyssolC DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(TARGETS libdyssol_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL(TARGETS libdyssol_static DESTINATION ${CMAKE_INSTALL_LIBDIR})

ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/Units")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/Solvers")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/DyssolMainWindow")

SET(INSTALL_DOCS_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/Dyssol/docs)

#===============================================
# Testing ======================================

ENABLE_TESTING()

SET(TESTS
  "Unit_Agglomerator_CellAverage"
  "Unit_Agglomerator_FFT"
  "Unit_Agglomerator_FixedPivot"
  "Unit_Bunker_Adaptive"
  "Unit_Bunker_Constant"
  "Unit_Crusher_BondBimodal"
  "Unit_Crusher_BondNormal"
  "Unit_Crusher_Cone"
  "Unit_Crusher_Const"
  "Unit_Crusher_PBMTM"
  "Unit_Granulator"
  "Unit_Mixer"
  "Unit_Screen_Molerus"
  "Unit_Screen_Multideck"
  "Unit_Screen_Plitt"
  "Unit_Screen_Probability"
  "Unit_Screen_Teipel"
  "Unit_Splitter"
  "Unit_TimeDelay_NormBased"
  "Unit_TimeDelay_SimpleShift"
  "Process_Agglomeration"
  "Process_Comminution"
  "Process_Granulation"
  "Process_SieveMill"
)

FOREACH(test ${TESTS})
  SET(CURRENT_TEST ${test})
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/tests/${CURRENT_TEST}/script.txt" "${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/script.txt")

  ADD_TEST(NAME ${CURRENT_TEST}_run
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          COMMAND ${CMAKE_BINDIR}/DyssolC --script=${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/script.txt
  )

  IF(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    FILE(READ ${CMAKE_SOURCE_DIR}/tests/${CURRENT_TEST}/tolerance TOLERANCE)

    ADD_TEST(NAME ${CURRENT_TEST}_diff
            COMMAND python3 ${CMAKE_SOURCE_DIR}/tests/compare.py "${CMAKE_SOURCE_DIR}/tests/${CURRENT_TEST}/ideal.res" "${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/real.res" -t ${TOLERANCE}
    )
    # _diff tests can only be run when the _run test is done
    SET_TESTS_PROPERTIES(${CURRENT_TEST}_diff PROPERTIES DEPENDS ${CURRENT_TEST}_run)
  ENDIF(NOT CMAKE_BUILD_TYPE MATCHES Debug)

ENDFOREACH(test ${TESTS})

#===============================================
# Documentation ================================
ADD_CUSTOM_TARGET(doc)
ADD_CUSTOM_COMMAND(
                  TARGET doc PRE_BUILD
                  COMMAND rm -rf ${CMAKE_BINARY_DIR}/doc/sphinx/_build
                  COMMAND sphinx-build -b html ${CMAKE_SOURCE_DIR}/DocumentationSphinx/ ${CMAKE_BINARY_DIR}/doc/sphinx/_build/html
                  COMMAND mkdir -p ${INSTALL_DOCS_PATH}/sphinx/
                  COMMAND cp -r ${CMAKE_BINARY_DIR}/doc/sphinx/_build/html ${INSTALL_DOCS_PATH}/sphinx/
                  )
