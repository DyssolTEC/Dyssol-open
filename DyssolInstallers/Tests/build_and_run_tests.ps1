# read arguments
$solution_dir       = $args[0]
$solution_path      = $args[1]
$platform           = $args[2]
$configuration      = $args[3]
$pre_build_binaries = $args[4]

# set variables
$build_path = "$($solution_dir)\build\windows"

# switch to x64 powershell if it is x86 now
if ($env:PROCESSOR_ARCHITEW6432 -eq "AMD64") {
	if ($myInvocation.Line) {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile $myInvocation.Line }
	else {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile -file "$($myInvocation.InvocationName)" $args }
	exit $lastexitcode
}

# compile projects
if ($pre_build_binaries -eq "true") {
	Write-Host Compiling $platform $configuration
	devenv $solution_path /build "$($configuration)|$($platform)"
}
	
# create build directory
if (!(Test-Path $build_path)) {
	New-Item -itemType Directory -Path $build_path
}

# compile docs
Set-Location $build_path
cmake $solution_dir -D BUILD_BINARIES=NO -D BUILD_DOCS=NO -A $platform -D CMAKE_BUILD_TYPE=$configuration --fresh
cmake --build . --target RUN_TESTS --parallel $ENV:NUMBER_OF_PROCESSORS
