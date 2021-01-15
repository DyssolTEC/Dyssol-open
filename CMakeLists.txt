cmake_minimum_required(VERSION 3.1.0)

project(Dyssol VERSION 0.10.0.2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(Dyssol
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.ui
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.rc
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Resources.qrc
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/main.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/AboutWindow.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CalculationSequenceEditor/CalculationSequenceEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CalculationSequenceEditor/CalculationSequenceEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/QtTable.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/QtTree.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/SignalBlocker.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/BaseModel.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/CalculationSequence.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/Flowsheet.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/FlowsheetParameters.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/ModelsManager.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/Simulator.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/SimulatorLog.cpp
    ${CMAKE_SOURCE_DIR}/SimulatorCore/Topology.cpp
)

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/CalculationSequenceEditor")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/SimulatorCore/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/ModelsAPI/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/BaseSolvers/")
target_link_libraries(Dyssol Qt5::Widgets)