# Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.
# Copyright (c) 2021, Anton Gladky <Anton.Gladky@iart.tu-freiberg.de>, IART, Technical University Bergakademie Freiberg

# TODO: option to disable shared libs

cmake_minimum_required(VERSION 3.1.0)

set (VERSION 1.1.1)
set (SOVERSION 1.0)

OPTION(BUILD_BINARIES "Whether to build binary files" ON)
OPTION(BUILD_DOCS "Whether to build documentation" ON)
OPTION(BUILD_TESTS "Whether to build tests" ON)

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

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##################################################
### Binaries 
IF(BUILD_BINARIES)

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

ENDIF(BUILD_BINARIES)

##################################################
### Testing 
IF(BUILD_TESTS)

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
    "Unit_Cyclone_Muschelknautz"
    "Unit_Granulator"
    "Unit_GranulatorSimpleBatch"
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

    IF(WIN32)
      ADD_TEST(NAME ${CURRENT_TEST}_run
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
              COMMAND ${CMAKE_SOURCE_DIR}/${CMAKE_GENERATOR_PLATFORM}/${CMAKE_BUILD_TYPE}/DyssolC --script=${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/script.txt
      )
    ELSE()
      ADD_TEST(NAME ${CURRENT_TEST}_run
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
              COMMAND ${CMAKE_BINDIR}/DyssolC --script=${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/script.txt
      )
    ENDIF(WIN32)

    IF(NOT CMAKE_BUILD_TYPE MATCHES Debug)
      FILE(READ ${CMAKE_SOURCE_DIR}/tests/${CURRENT_TEST}/tolerance TOLERANCE)

      IF(WIN32)
        ADD_TEST(NAME ${CURRENT_TEST}_diff
                COMMAND python  ${CMAKE_SOURCE_DIR}/tests/compare.py "${CMAKE_SOURCE_DIR}/tests/${CURRENT_TEST}/ideal.res" "${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/real.res" -t ${TOLERANCE}
        )
      ELSE()
        ADD_TEST(NAME ${CURRENT_TEST}_diff
                COMMAND python3 ${CMAKE_SOURCE_DIR}/tests/compare.py "${CMAKE_SOURCE_DIR}/tests/${CURRENT_TEST}/ideal.res" "${CMAKE_BINARY_DIR}/tests/${CURRENT_TEST}/real.res" -t ${TOLERANCE}
        )
      ENDIF(WIN32)
      
      # _diff tests can only be run when the _run test is done
      SET_TESTS_PROPERTIES(${CURRENT_TEST}_diff PROPERTIES DEPENDS ${CURRENT_TEST}_run)
    ENDIF(NOT CMAKE_BUILD_TYPE MATCHES Debug)

  ENDFOREACH(test ${TESTS})

ENDIF(BUILD_TESTS)

##################################################
### Documentation 
IF(BUILD_DOCS)

  #=================================================
  #== Doxygen

  FIND_PACKAGE(Doxygen)

  # List of all API headers
  SET(API_FILE_NAMES
    "MaterialsDatabase/DefinesMDB"
    "ModelsAPI/BaseUnit"
    "ModelsAPI/ChemicalReaction"
    "ModelsAPI/DenseMDMatrix"
    "ModelsAPI/DistributionsFunctions"
    "ModelsAPI/Holdup"
    "ModelsAPI/Matrix2D"
    "ModelsAPI/MixtureEnthalpyLookup"
    "ModelsAPI/PlotManager"
    "ModelsAPI/StateVariable"
    "ModelsAPI/Stream"
    "ModelsAPI/TransformMatrix"
    "ModelsAPI/UnitParameters"
    "ModelsAPI/UnitPorts"
    "BaseSolvers/AgglomerationSolver"
    "BaseSolvers/BaseSolver"
    "Utilities/DyssolDefines"
  )

  # Gather all API headers
  FOREACH(FILENAME ${API_FILE_NAMES})
    FILE(GLOB_RECURSE HEADER ${CMAKE_SOURCE_DIR}/${FILENAME}.h)
    LIST(APPEND API_HEADERS ${HEADER})
  ENDFOREACH(FILENAME ${API_FILE_NAMES})
  # Format the list for Doxygen file
  STRING(REPLACE ";" "\" \"" API_HEADERS_LIST "${API_HEADERS}")

  # Setup Doxygen variables
  SET(DOXYGEN_INPUT_LIST ${API_HEADERS_LIST})
  SET(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/doc/doxygen)
  SET(DOXYGEN_OUTPUT_XML_DIR ${DOXYGEN_OUTPUT_DIR}/xml)
  SET(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_XML_DIR}/index.xml)
  SET(DOXYFILE_IN ${CMAKE_SOURCE_DIR}/Documentation/Doxyfile.in)
  SET(DOXYFILE_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

  # Replace variables inside @@ with the current values
  CONFIGURE_FILE(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  # Create Doxygen output directory
  FILE(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

  # Build with Doxygen
  ADD_CUSTOM_COMMAND(OUTPUT ${DOXYGEN_INDEX_FILE}
                    DEPENDS ${API_HEADERS}
                    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                    MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                    COMMENT "Generating Doxygen files"
                    VERBATIM
  )
  ADD_CUSTOM_TARGET(doxygen DEPENDS ${DOXYGEN_INDEX_FILE})

  #=================================================
  #== Sphinx

  FIND_PACKAGE(Sphinx)

  SET(SPHINX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Documentation/)
  SET(SPHINX_BUILD ${CMAKE_BINARY_DIR}/doc/sphinx/_build/html)
  SET(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)
  SET(SPHINX_CONFIG_IN ${SPHINX_SOURCE_DIR}/conf.py.in)
  SET(SPHINX_CONFIG_OUT ${SPHINX_SOURCE_DIR}/conf.py)

  # Replace variables inside @@ with the current values
  CONFIGURE_FILE(${SPHINX_CONFIG_IN} ${SPHINX_CONFIG_OUT} @ONLY)
  
  ADD_CUSTOM_COMMAND(OUTPUT ${SPHINX_INDEX_FILE}
                    COMMAND ${SPHINX_EXECUTABLE} -b linkcheck  ${SPHINX_SOURCE_DIR} ${SPHINX_BUILD} # Check links
                    COMMAND ${SPHINX_EXECUTABLE} -b html -Dbreathe_projects.Dyssol=${DOXYGEN_OUTPUT_XML_DIR} ${SPHINX_SOURCE_DIR} ${SPHINX_BUILD}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    DEPENDS ${SPHINX_SOURCE_DIR}/index.rst ${DOXYGEN_INDEX_FILE}
                    MAIN_DEPENDENCY ${SPHINX_SOURCE_DIR}/conf.py
                    COMMENT "Generating documentation with Sphinx"
  )
  ADD_CUSTOM_TARGET(sphinx DEPENDS ${SPHINX_INDEX_FILE})

  ADD_CUSTOM_TARGET(doc DEPENDS sphinx)

  # Add an install target to install the docs
  INSTALL(DIRECTORY ${SPHINX_BUILD}
          DESTINATION ${INSTALL_DOCS_PATH}/sphinx/
  )

ENDIF(BUILD_DOCS)
