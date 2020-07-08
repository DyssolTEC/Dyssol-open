:: Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

@echo off
setlocal EnableDelayedExpansion

call %cd%\windows_files.bat

:: ================================================================
:: Core libraries

for %%a in (%CORE_DIRS%) do (
	copy ..\%%a\*.cpp .\%CORE_DIR%
	copy ..\%%a\*.c .\%CORE_DIR%
	copy ..\%%a\*.h .\%CORE_DIR%
)

:: ================================================================
:: Units

for %%a in (%UNITS_DIRS%) do (
	mkdir .\%UNITS_DIR%\%%a
	copy ..\Units\%%a\*.cpp .\%UNITS_DIR%\%%a
	copy ..\Units\%%a\*.c .\%UNITS_DIR%\%%a
	copy ..\Units\%%a\*.h .\%UNITS_DIR%\%%a
)

:: ================================================================
:: Solvers

for %%a in (%SOLVERS_DIRS%) do (
	mkdir .\%SOLVERS_DIR%\%%a
	copy ..\Solvers\%%a\*.cpp .\%SOLVERS_DIR%\%%a
	copy ..\Solvers\%%a\*.c .\%SOLVERS_DIR%\%%a
	copy ..\Solvers\%%a\*.h .\%SOLVERS_DIR%\%%a
)