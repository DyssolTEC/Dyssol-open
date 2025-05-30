# Copyright (c) 2020, Dyssol Development Team. 
# Copyright (c) 2021, Anton Gladky <Anton.Gladky@iart.tu-freiberg.de>, IART, Technical University Bergakademie Freiberg.
# Copyright (c) 2024, DyssolTEC GmbH. 
# All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

set(UnitsNames
    "Agglomerator"
    "Bunker"
    "Crusher"
    "CrusherPBMTM"
    "CycloneMuschelknautz"
    "Granulator"
    "GranulatorSimpleBatch"
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


foreach(foldername ${UnitsNames})
    file(GLOB_RECURSE src ${CMAKE_SOURCE_DIR}/Units/${foldername}/*.cpp)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Units/${foldername}/)
    add_library(${foldername} SHARED ${src})
    INSTALL(TARGETS ${foldername} DESTINATION ${INSTALL_UNITS_LIB_PATH})
endforeach(foldername ${UnitsNames})
