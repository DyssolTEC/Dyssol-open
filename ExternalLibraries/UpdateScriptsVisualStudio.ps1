# Copyright (c) 2025, Dyssol Development Team. 
# Copyright (c) 2025, DyssolTEC GmbH. 
# All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

# Updates Visual Studio version in all ps1 scripts

# Get the full path of the current script
$currentScript = $MyInvocation.MyCommand.Definition

# Get all .ps files in the current directory, excluding the current script
$scriptFiles = Get-ChildItem -Path . -Filter *.ps1 -File | Where-Object {
    $_.FullName -ne $currentScript
}

# Process files
foreach ($file in $scriptFiles) {
    Write-Host "Processing file: $($file.Name)"

    # Read content of the file
    $content = Get-Content -Path $file.FullName

    # Modify content
    $updatedContent = $content -replace 'Visual Studio 16 2019', 'Visual Studio 17 2022'

    # Overwrite the file with updated content
    Set-Content -Path $file.FullName -Value $updatedContent
}

Write-Host "Update completed"
