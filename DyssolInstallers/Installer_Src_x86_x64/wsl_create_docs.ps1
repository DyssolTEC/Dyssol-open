# variables
$src_path = "..\.."
$src_path_wsl = "../.."
$install_path = "install"
$build_path = "build"
$current_path  = (Get-Item -Path ".\" -Verbose).FullName

# switch to x64 powershell if it is x86 now
if ($env:PROCESSOR_ARCHITEW6432 -eq "AMD64") {
	if ($myInvocation.Line) {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile $myInvocation.Line }
	else {
		&"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile -file "$($myInvocation.InvocationName)" $args }
	exit $lastexitcode
}

# create build and install directories
if (!(Test-Path $src_path\$build_path)) {
	New-Item -itemType Directory -Path $src_path\$build_path
}
if (!(Test-Path $src_path\$install_path)) {
	New-Item -itemType Directory -Path $src_path\$install_path
}

#install dependencies
#Write-Host "Install dependencies via WSL"
#bash -c "pip install -U sphinx sphinx-rtd-theme breathe"
#"apt-get install doxygen graphviz &&  apt update && apt install python3-pip && 
# compile docs
Write-Host "Generating documentation via WSL"
wsl -e bash -lic "cd $src_path_wsl/$build_path; cmake .. -DCMAKE_INSTALL_PREFIX=../$install_path -DBUILD_BINARIES=NO; make doc; make install"
#bash -c "cd $src_path_wsl/$build_path && cmake .. -DCMAKE_INSTALL_PREFIX=../$install_path -DBUILD_BINARIES=NO && make doc && make install"

if(Test-Path $src_path\$install_path\share\Dyssol\docs\sphinx\html) {
	Write-Host "Documentation generated successfully."
}
else {
	Write-Warning "No documents generated."
	exit 1
}