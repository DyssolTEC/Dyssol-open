find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(DyssolGUI
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/AboutWindow.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.rc
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.ui
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/main.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Resources.qrc
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/SaveLoadThread.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CalculationSequenceEditor/CalculationSequenceEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CalculationSequenceEditor/CalculationSequenceEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CompoundsManager/CompoundsManager.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CompoundsManager/CompoundsManager.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/DustFormationTesterTab/DustFormationTesterTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/DustFormationTesterTab/DustFormationTesterTab.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/FlowsheetEditor/FlowsheetEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/FlowsheetEditor/FlowsheetEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/DimensionParameters.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/DimensionParameters.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/GridEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/GridEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/HoldupsEditor/HoldupsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/HoldupsEditor/HoldupsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/CorrelationEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/CorrelationEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/DescriptionEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/DescriptionEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/MaterialsDatabaseTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/MaterialsDatabaseTab.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyAdder.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyAdder.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/ModulesManagerTab/ModulesManagerTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/ModulesManagerTab/ModulesManagerTab.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/OptionsEditor/OptionsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/OptionsEditor/OptionsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/PhasesEditor/PhasesEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/PhasesEditor/PhasesEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SettingsEditor/SettingsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SettingsEditor/SettingsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/ProgressThread.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/SimulatorTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/SimulatorTab.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/StreamsViewer/StreamsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/StreamsViewer/StreamsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/TearStreamsEditor/TearStreamsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/TearStreamsEditor/TearStreamsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/PlotsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/PlotsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/UnitsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/UnitsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/BasicStreamEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/BasicStreamEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/DDTable.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/DistrFunctionDialog.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/Functional.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/MDMTable.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/ParamsItem.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/SolidDistributionsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/SolidDistributionsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamsViewer/BasicStreamsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamsViewer/BasicStreamsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicThread/BasicThread.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/GridLimitsDialog.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/GridLimitsDialog.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/QtPlot.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/QtTable.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/QtTree.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/SignalBlocker.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/StatusWindow/StatusWindow.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/StatusWindow/StatusWindow.ui
    )

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/CalculationSequenceEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/CompoundsManager/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/DustFormationTesterTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/FlowsheetEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/HoldupsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/ModulesManagerTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/OptionsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/PhasesEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/SettingsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/StreamsViewer//")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/TearStreamsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamsViewer/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/BasicThread/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/StatusWindow/")


target_compile_definitions(DyssolGUI PRIVATE INSTALL_UNITS_LIB_PATH="${INSTALL_UNITS_LIB_PATH}")

TARGET_LINK_LIBRARIES(DyssolGUI Qt5::Widgets libdyssol_shared)
INSTALL(TARGETS DyssolGUI DESTINATION ${CMAKE_INSTALL_BINDIR})