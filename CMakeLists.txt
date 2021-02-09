cmake_minimum_required(VERSION 3.1.0)

project(Dyssol VERSION 0.10.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

INCLUDE(GNUInstallDirs)

SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


add_executable(DyssolC
    ${CMAKE_SOURCE_DIR}/DyssolConsole/ConfigFileParser.cpp
    ${CMAKE_SOURCE_DIR}/DyssolConsole/main.cpp
)

set(libdyssolsrc
    ${CMAKE_SOURCE_DIR}/BaseSolvers/AgglomerationSolver.cpp
    ${CMAKE_SOURCE_DIR}/BaseSolvers/BaseSolver.cpp
    ${CMAKE_SOURCE_DIR}/BaseSolvers/PBMSolver.cpp
    ${CMAKE_SOURCE_DIR}/CacheHandler/BaseCacheHandler.cpp
    ${CMAKE_SOURCE_DIR}/CacheHandler/DenseDistrCacher.cpp
    ${CMAKE_SOURCE_DIR}/CacheHandler/MDMatrCacher.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/DAEModel.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/DAESolver.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/NLModel.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/NLSolver.cpp
    ${CMAKE_SOURCE_DIR}/HDF5Handler/H5Handler.cpp
    ${CMAKE_SOURCE_DIR}/HDF5Handler/PacketTable.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/BaseProperty.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/Compound.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/ConstProperty.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/Correlation.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/Interaction.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/MaterialsDatabase.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/TPDProperty.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/BaseUnit.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/DenseDistr2D.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/DenseMDMatrix.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/DependentValues.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/DistributionsGrid.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/DynamicUnit.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/Holdup.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/LookupTable.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/MaterialStream.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/Matrix2D.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/MDMatrix.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/SteadyStateUnit.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/Stream.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/Stream.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/TDArray.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/TransformMatrix.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/UnitParameters.cpp
    ${CMAKE_SOURCE_DIR}/Modules/DustFormationTester/DustFormationTester.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/BaseModel.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/CalculationSequence.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/Flowsheet.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/FlowsheetParameters.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/ModelsManager.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/Simulator.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/SimulatorLog.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/Topology.cpp
    ${CMAKE_SOURCE_DIR}/Utilities/FileSystem.cpp
    ${CMAKE_SOURCE_DIR}/Utilities/StringFunctions.cpp
    ${CMAKE_SOURCE_DIR}/Utilities/ThreadPool.cpp
)

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/BaseSolvers/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/CacheHandler/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/EquationSolvers/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/ExternalLibraries/sundials/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/HDF5Handler/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/MaterialsDatabase/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/ModelsAPI/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/Modules/DustFormationTester/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/SimulatorCore/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/Utilities/")

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

SET(INSTALL_UNITS_LIB_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/Dysssol/Units)
SET(INSTALL_MATERIALDB_PATH  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/Dysssol/MaterialsDB)

CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/DyssolMainWindow/config.ini.in" "${CMAKE_BINARY_DIR}/config.ini")
INSTALL(FILES "${CMAKE_BINARY_DIR}/config.ini" DESTINATION ${CMAKE_INSTALL_DATADIR})

INSTALL(FILES "${CMAKE_SOURCE_DIR}/Materials.dmdb" DESTINATION ${INSTALL_MATERIALDB_PATH})

INSTALL(TARGETS DyssolC DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(TARGETS libdyssol_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL(TARGETS libdyssol_static DESTINATION ${CMAKE_INSTALL_LIBDIR})

ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/Units")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/DyssolMainWindow")