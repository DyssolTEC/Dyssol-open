:: Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

@echo off
setlocal EnableDelayedExpansion

call %cd%\windows_files.bat

:: ================================================================
:: Core libraries

for %%a in (%CORE_DIRS%) do (
	call :link_core %%a
)

:: ================================================================
:: Units

for %%a in (%UNITS_DIRS%) do (
	call :link_models %%a
)

:: ================================================================
:: Solvers

for %%a in (%SOLVERS_DIRS%) do (
	call :link_solvers %%a
)

:: ================================================================
:: Functions

goto :eof

:link_core
	for /R ..\%1\ %%f in (*.cpp *.c *.h) do (
		set FILE_DIR=%%f
		mklink !FILE_DIR:\%1\=\DyssolLinux\%CORE_DIR%\! %%f
	)
goto :eof

:link_models
	mkdir .\%UNITS_DIR%\%1
	for /R ..\Units\%1\ %%f in (*.cpp *.c *.h) do (
		set FILE_DIR=%%f
		mklink !FILE_DIR:\Units\%1\=\DyssolLinux\%UNITS_DIR%\%1\! %%f
)
goto :eof

:link_solvers
	mkdir .\%SOLVERS_DIR%\%1
	for /R ..\Solvers\%1\ %%f in (*.cpp *.c *.h) do (
		set FILE_DIR=%%f
		mklink !FILE_DIR:\Solvers\%1\=\DyssolLinux\%SOLVERS_DIR%\%1\! %%f
)
goto :eof