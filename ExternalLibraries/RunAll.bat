:: Copyright (c) 2020, Dyssol Development Team. 
:: Copyright (c) 2024, DyssolTEC GmbH. 
:: All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

powershell -executionpolicy bypass -File .\CompileSundials.ps1
powershell -executionpolicy bypass -File .\CompileZLib.ps1
powershell -executionpolicy bypass -File .\CompileHDF5.ps1
powershell -executionpolicy bypass -File .\CompileGraphviz.ps1