:: Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

@echo off
set DATE_TIME=
for /f "skip=1 delims=" %%x in ('WMIC OS GET LocalDateTime') do if not defined DATE_TIME set DATE_TIME=%%x

set DATE.YEAR=%DATE_TIME:~2,2%
set DATE.MONTH=%DATE_TIME:~4,2%
set DATE.DAY=%DATE_TIME:~6,2%
set DATE.HOUR=%DATE_TIME:~8,2%
set DATE.MINUTE=%DATE_TIME:~10,2%
set DATE.SECOND=%DATE_TIME:~12,2%

set DATE_TIME_F=%DATE.YEAR%%DATE.MONTH%%DATE.DAY%.%DATE.HOUR%%DATE.MINUTE%%DATE.SECOND%

for /f %%i in ('git rev-parse --short HEAD') do set CURR_COMMIT_HASH=%%i

(
echo #pragma once 
echo const char CURRENT_BUILD_VERSION[] = "%DATE_TIME_F%.%CURR_COMMIT_HASH%";
) > %~dp0\..\Utilities\BuildVersion.h