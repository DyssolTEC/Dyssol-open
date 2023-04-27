# variables
$src_path = "..\.."
$install_path = "$($src_path)\install\windows"
$build_path = "$($src_path)\build\windows"

# switch to x64 powershell if it is x86 now
if ($env:PROCESSOR_ARCHITEW6432 -eq "AMD64") {
	if ($myInvocation.Line) {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile $myInvocation.Line }
	else {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile -file "$($myInvocation.InvocationName)" $args }
	exit $lastexitcode
}

# create build and install directories
if (!(Test-Path $build_path)) {
	New-Item -itemType Directory -Path $build_path
}
if (!(Test-Path $install_path)) {
	New-Item -itemType Directory -Path $install_path
}

# compile docs
Set-Location $build_path
cmake $src_path -DCMAKE_INSTALL_PREFIX="$($install_path)" -DBUILD_BINARIES=NO
cmake --build . --parallel --target doc
cmake --build . --parallel --target INSTALL
