# Updates Visual Studio version in all compilation scripts

$filestoUpdate = @(
    "CompileZLib.ps1",
    "CompileSundials.ps1",
    "CompileHDF5.ps1",
    "CompileGraphviz.ps1"
)

foreach ($file in $filestoUpdate) {
    if (Test-Path $file) {
        Write-Host "Updating $file..."
        $content = Get-Content $file
        $content = $content -replace "Visual Studio 16 2019", "Visual Studio 17 2022"
        $content | Set-Content $file -Force
    } else {
        Write-Host "Warning: $file not found"
    }
}

Write-Host "Update completed"