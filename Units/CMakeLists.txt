

set(UnitsNames
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


foreach(foldername ${UnitsNames})
    file(GLOB_RECURSE src ${CMAKE_SOURCE_DIR}/Units/${foldername}/*.cpp)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Units/${foldername}/)
    add_library(${foldername} SHARED ${src})
    INSTALL(TARGETS ${foldername} DESTINATION ${INSTALL_UNITS_LIB_PATH})
endforeach(foldername ${UnitsNames})
