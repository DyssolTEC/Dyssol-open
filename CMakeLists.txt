cmake_minimum_required(VERSION 3.1.0)

project(Dyssol VERSION 0.10.0.2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(Dyssol
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.ui
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.rc
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Resources.qrc
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/main.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/Dyssol.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/AboutWindow.cpp
    ${CMAKE_SOURCE_DIR}/DyssolMainWindow/SaveLoadThread.cpp
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
    ${CMAKE_SOURCE_DIR}/HDF5Handler/H5Handler.cpp
    ${CMAKE_SOURCE_DIR}/HDF5Handler/PacketTable.cpp
    ${CMAKE_SOURCE_DIR}/Utilities/FileSystem.cpp
    ${CMAKE_SOURCE_DIR}/Utilities/StringFunctions.cpp
    ${CMAKE_SOURCE_DIR}/Utilities/ThreadPool.cpp
    ${CMAKE_SOURCE_DIR}/CacheHandler/BaseCacheHandler.cpp
    ${CMAKE_SOURCE_DIR}/CacheHandler/DenseDistrCacher.cpp
    ${CMAKE_SOURCE_DIR}/CacheHandler/MDMatrCacher.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/BaseProperty.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/Compound.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/ConstProperty.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/Correlation.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/Interaction.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/MaterialsDatabase.cpp
    ${CMAKE_SOURCE_DIR}/MaterialsDatabase/TPDProperty.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/CorrelationEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/DescriptionEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/MaterialsDatabaseTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyAdder.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/CorrelationEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/DescriptionEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/MaterialsDatabaseTab.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyAdder.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/PropertyEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/GridLimitsDialog.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/GridLimitsDialog.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/QtPlot.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CompoundsManager/CompoundsManager.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/CompoundsManager/CompoundsManager.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/FlowsheetEditor/FlowsheetEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/FlowsheetEditor/FlowsheetEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/DimensionParameters.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/DimensionParameters.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/GridEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/GridEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/HoldupsEditor/HoldupsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/HoldupsEditor/HoldupsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/BasicStreamEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/BasicStreamEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/DDTable.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/DistrFunctionDialog.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/Functional.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/MDMTable.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/ParamsItem.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/SolidDistributionsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/SolidDistributionsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/ModulesManagerTab/ModulesManagerTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/ModulesManagerTab/ModulesManagerTab.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/OptionsEditor/OptionsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/OptionsEditor/OptionsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/PhasesEditor/PhasesEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/PhasesEditor/PhasesEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/ProgressThread.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/SimulatorTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/SimulatorTab.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicThread/BasicThread.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/StatusWindow/StatusWindow.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/StatusWindow/StatusWindow.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/StreamsViewer/StreamsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/StreamsViewer/StreamsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamsViewer/BasicStreamsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamsViewer/BasicStreamsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/PlotsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/PlotsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/UnitsViewer.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/UnitsViewer.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/TearStreamsEditor/TearStreamsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/TearStreamsEditor/TearStreamsEditor.ui
    ${CMAKE_SOURCE_DIR}/GUIDialogs/DustFormationTesterTab/DustFormationTesterTab.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/DustFormationTesterTab/DustFormationTesterTab.ui
    ${CMAKE_SOURCE_DIR}/Modules/DustFormationTester/DustFormationTester.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SettingsEditor/SettingsEditor.cpp
    ${CMAKE_SOURCE_DIR}/GUIDialogs/SettingsEditor/SettingsEditor.ui
    ${CMAKE_SOURCE_DIR}/ModelsAPI/Stream.cpp
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
    ${CMAKE_SOURCE_DIR}/ModelsAPI/TDArray.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/TransformMatrix.cpp
    ${CMAKE_SOURCE_DIR}/ModelsAPI/UnitParameters.cpp
    ${CMAKE_SOURCE_DIR}/BaseSolvers/BaseSolver.cpp
    ${CMAKE_SOURCE_DIR}/BaseSolvers/PBMSolver.cpp
    ${CMAKE_SOURCE_DIR}/BaseSolvers/AgglomerationSolver.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/DAEModel.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/DAESolver.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/NLModel.cpp
    ${CMAKE_SOURCE_DIR}/EquationSolvers/NLSolver.cpp
    )

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/CalculationSequenceEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/QtWidgets/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/SimulatorCore/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/ModelsAPI/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/BaseSolvers/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/HDF5Handler/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/Utilities/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/CacheHandler/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/MaterialsDatabase/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/MaterialsDatabaseTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/QtPlot/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/CompoundsManager/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/FlowsheetEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/GridEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/HoldupsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/ModulesManagerTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/OptionsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/PhasesEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/SimulatorTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/BasicThread/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/StatusWindow/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/StreamsViewer//")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIWidgets/BasicStreamsViewer/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/UnitsViewer/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/TearStreamsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/DustFormationTesterTab/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/Modules/DustFormationTester/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/GUIDialogs/SettingsEditor/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/EquationSolvers/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/DyssolLinux/sundials_install/include/nvector/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/DyssolLinux/sundials_install/include/ida/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/DyssolLinux/sundials_install/include/kinsol/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/DyssolLinux/sundials_install/include/sundials/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/DyssolLinux/sundials_install/include/sunlinsol/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/DyssolLinux/sundials_install/include/sunmatrix/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/ExternalLibraries/sundials/include/")


INCLUDE_DIRECTORIES("/usr/include/hdf5/serial/")
TARGET_LINK_LIBRARIES(Dyssol Qt5::Widgets pthread dl hdf5_serial hdf5_serial_cpp hdf5_serial_hl hdf5_serial_hl_cpp z sundials_ida sundials_kinsol)


ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/Units")