# read arguments
$solution_dir = $args[0]

# set variables
$install_path = "$($solution_dir)\install\windows"
$build_path = "$($solution_dir)\build\windows"

# switch to x64 powershell if it is x86 now
if ($env:PROCESSOR_ARCHITEW6432 -eq "AMD64") {
	if ($myInvocation.Line) {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile $myInvocation.Line }
	else {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile -file "$($myInvocation.InvocationName)" $args }
	exit $lastexitcode
}

# remove old install directory
Remove-Item -Recurse -Force $install_path -ErrorAction Ignore

# create build and install directories
if (!(Test-Path $build_path)) {
	New-Item -itemType Directory -Path $build_path
}
if (!(Test-Path $install_path)) {
	New-Item -itemType Directory -Path $install_path
}

# compile docs
Set-Location $build_path
cmake $solution_dir -D CMAKE_INSTALL_PREFIX="$($install_path)" -D BUILD_BINARIES=NO -D BUILD_TESTS=NO --fresh
cmake --build . --parallel --target doc
cmake --build . --parallel --target INSTALL
