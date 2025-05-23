# Copyright (c) 2020, Dyssol Development Team. 
# Copyright (c) 2021, Anton Gladky <Anton.Gladky@iart.tu-freiberg.de>, IART, Technical University Bergakademie Freiberg
# Copyright (c) 2024, DyssolTEC GmbH. 
# All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

find_package(Qt6 COMPONENTS Widgets)
if (NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

set(DyssolGUInames
	"DyssolGUI"
	"GUIDialogs"
	"GUIWidgets"
)

foreach(foldername ${DyssolGUInames})
    file(GLOB_RECURSE src ${CMAKE_SOURCE_DIR}/${foldername}/*.cpp)
    set(DyssolGUIsrc ${DyssolGUIsrc} ${src})
    file(GLOB_RECURSE src ${CMAKE_SOURCE_DIR}/${foldername}/*.rc)
    set(DyssolGUIsrc ${DyssolGUIsrc} ${src})
    file(GLOB_RECURSE src ${CMAKE_SOURCE_DIR}/${foldername}/*.ui)
    set(DyssolGUIsrc ${DyssolGUIsrc} ${src})
    file(GLOB_RECURSE src ${CMAKE_SOURCE_DIR}/${foldername}/*.qrc)
    set(DyssolGUIsrc ${DyssolGUIsrc} ${src})
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/${foldername}/)
    SUBDIRLIST(DyssolGUI_folders ${CMAKE_SOURCE_DIR}/${foldername}/)
    FOREACH(subdir ${DyssolGUI_folders})
        INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/${foldername}/${subdir}/)
    ENDFOREACH()
endforeach(foldername ${DyssolGUInames})

add_executable(DyssolGUI ${DyssolGUIsrc})

target_compile_definitions(DyssolGUI PRIVATE INSTALL_DOCS_PATH="${INSTALL_DOCS_PATH}")
target_compile_definitions(DyssolGUI PRIVATE INSTALL_CONFIG_PATH="${INSTALL_CONFIG_PATH}")

TARGET_LINK_LIBRARIES(DyssolGUI Qt::Widgets libdyssol_shared)
# link filesystem or experimental/filesystem for gcc
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  IF (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
    TARGET_LINK_LIBRARIES(DyssolGUI -lstdc++fs)
  ENDIF()
ENDIF()

INSTALL(TARGETS DyssolGUI DESTINATION ${CMAKE_INSTALL_BINDIR})