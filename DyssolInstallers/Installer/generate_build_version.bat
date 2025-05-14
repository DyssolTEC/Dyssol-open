:: Copyright (c) 2025, DyssolTEC GmbH. 
:: All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

@echo off
setlocal

:: Run the ps1 script with the same name as this one
set "script_name=%~n0"
set "ps1_script=%~dp0%script_name%.ps1"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ps1_script%"

endlocal
pause
