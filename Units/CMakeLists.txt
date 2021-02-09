
add_library(Agglomerator SHARED
    ${CMAKE_SOURCE_DIR}/Units/Agglomerator/Agglomerator.cpp
    )
INSTALL(TARGETS Agglomerator DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Bunker SHARED
    ${CMAKE_SOURCE_DIR}/Units/Bunker/Bunker.cpp
    )
INSTALL(TARGETS Bunker DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Crusher SHARED
    ${CMAKE_SOURCE_DIR}/Units/Crusher/Crusher.cpp
    )
INSTALL(TARGETS Crusher DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(CrusherPBMTM SHARED
    ${CMAKE_SOURCE_DIR}/Units/CrusherPBMTM/CrusherPBMTM.cpp
    )
INSTALL(TARGETS CrusherPBMTM DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Cyclone2 SHARED
    ${CMAKE_SOURCE_DIR}/Units/Cyclone2/Cyclone2.cpp
)
INSTALL(TARGETS CrusherPBMTM DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Granulator SHARED
    ${CMAKE_SOURCE_DIR}/Units/Granulator/Granulator.cpp
)
INSTALL(TARGETS CrusherPBMTM DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(HeatExchanger SHARED
    ${CMAKE_SOURCE_DIR}/Units/HeatExchanger/HeatExchanger.cpp
)
INSTALL(TARGETS CrusherPBMTM DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Inlet SHARED
    ${CMAKE_SOURCE_DIR}/Units/Inlet/Inlet.cpp
)
INSTALL(TARGETS Inlet DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Mixer SHARED
    ${CMAKE_SOURCE_DIR}/Units/Mixer/Mixer.cpp
)
INSTALL(TARGETS Mixer DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Mixer3 SHARED
    ${CMAKE_SOURCE_DIR}/Units/Mixer3/Mixer3.cpp
)
INSTALL(TARGETS Mixer3 DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Outlet SHARED
    ${CMAKE_SOURCE_DIR}/Units/Outlet/Outlet.cpp
)
INSTALL(TARGETS Outlet DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Screen SHARED
    ${CMAKE_SOURCE_DIR}/Units/Screen/Screen.cpp
)
INSTALL(TARGETS Screen DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Splitter SHARED
    ${CMAKE_SOURCE_DIR}/Units/Splitter/Splitter.cpp
)
INSTALL(TARGETS Splitter DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(Splitter3 SHARED
    ${CMAKE_SOURCE_DIR}/Units/Splitter3/Splitter3.cpp
)
INSTALL(TARGETS Splitter3 DESTINATION ${INSTALL_UNITS_LIB_PATH})

add_library(TimeDelay SHARED
    ${CMAKE_SOURCE_DIR}/Units/TimeDelay/TimeDelay.cpp
)
INSTALL(TARGETS TimeDelay DESTINATION ${INSTALL_UNITS_LIB_PATH})
