# Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.
# Copyright (c) 2021, Anton Gladky <Anton.Gladky@iart.tu-freiberg.de>, IART, Technical University Bergakademie Freiberg

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

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
	"DyssolMainWindow"
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

TARGET_LINK_LIBRARIES(DyssolGUI Qt5::Widgets libdyssol_shared)
INSTALL(TARGETS DyssolGUI DESTINATION ${CMAKE_INSTALL_BINDIR})