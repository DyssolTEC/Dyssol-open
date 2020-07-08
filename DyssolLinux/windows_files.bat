:: Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

set CORE_DIR=src\Core
set UNITS_DIR=src\Units
set SOLVERS_DIR=src\Solvers

rmdir /S /Q .\%CORE_DIR%
mkdir .\%CORE_DIR%

rmdir /S /Q .\%UNITS_DIR%
mkdir .\%UNITS_DIR%

rmdir /S /Q .\%SOLVERS_DIR%
mkdir .\%SOLVERS_DIR%

:: ================================================================
:: Core libraries

set CORE_DIRS=^
BaseSolvers ^
CacheHandler ^
EquationSolvers ^
HDF5Handler ^
MaterialsDatabase ^
ModelsAPI ^
SimulatorCore ^
Utilities ^
DyssolConsole

:: ================================================================
:: Units

set UNITS_DIRS=^
Agglomerator ^
Bunker ^
Crusher ^
CrusherPBMTM ^
Cyclone2 ^
Granulator ^
HeatExchanger ^
Inlet ^
Mixer ^
Mixer3 ^
Outlet ^
Screen ^
Splitter ^
Splitter3 ^
TimeDelay

:: ================================================================
:: Solvers

set SOLVERS_DIRS=^
AgglomerationCellAverage ^
AgglomerationFixedPivot ^
AgglomerationFFT
