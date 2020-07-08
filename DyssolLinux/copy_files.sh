#!/bin/bash
# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

CORE_DIR=./src/Core
UNITS_DIR=./src/Units
SOLVERS_DIR=./src/Solvers

# ================================================================
# Core libraries

CORE_DIRS=(
	BaseSolvers
	CacheHandler
	EquationSolvers
	HDF5Handler
	MaterialsDatabase
	ModelsAPI
	SimulatorCore
	Utilities
	DyssolConsole
)

# ================================================================
# Units

UNITS_DIRS=(
	Agglomerator
	Bunker
	Crusher
	CrusherPBMTM
	Cyclone2
	Granulator
	HeatExchanger
	Inlet
	Mixer
	Mixer3
	Outlet
	Screen
	Splitter
	Splitter3
	TimeDelay
)

# ================================================================
# Solvers

SOLVERS_DIRS=(
	AgglomerationCellAverage
	AgglomerationFixedPivot
	AgglomerationFFT
)

# ================================================================
# Copy files

rm -rf $CORE_DIR
mkdir -p $CORE_DIR
for DIR in "${CORE_DIRS[@]}"; do 
	find $PWD/../$DIR    -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.c' -o -name '*.h' \) -exec cp '{}' ''$PWD'/'$CORE_DIR'' ';'
done

rm -rf $UNITS_DIR
mkdir -p $UNITS_DIR
for DIR in "${UNITS_DIRS[@]}"; do 
	mkdir -p $UNITS_DIR/$DIR
	find $PWD/../Units/$DIR   -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.c' -o -name '*.h' \) -exec cp '{}' ''$PWD'/'$UNITS_DIR'/'$DIR'' ';'
done

rm -rf $SOLVERS_DIR
mkdir -p $SOLVERS_DIR
for DIR in "${SOLVERS_DIRS[@]}"; do 
	mkdir -p $SOLVERS_DIR/$DIR
	find $PWD/../Solvers/$DIR -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.c' -o -name '*.h' \) -exec cp '{}' ''$PWD'/'$SOLVERS_DIR'/'$DIR'' ';'
done
