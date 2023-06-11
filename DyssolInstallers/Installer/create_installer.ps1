# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

### read arguments

$solution_dir   = $args[0]
$solution_path  = $args[1]
$qt_install_dir = $args[2]
$platforms      = $args[3]
$documentation  = $args[4]
$sdk_type       = $args[5]

Write-Host $platforms

### compile projects

if ($platforms -eq "Win32" -Or $platforms -eq "Both" -Or $platforms -eq "") {
	Write-Host Compiling Release Win32
	devenv $solution_path /build "Release|Win32"
	Write-Host Compiling Debug Win32
	devenv $solution_path /build "Debug|Win32"
}
if ($platforms -eq "x64" -Or $platforms -eq "Both" -Or $platforms -eq "") {
	Write-Host Compiling Release x64
	devenv $solution_path /build "Release|x64"
	Write-Host Compiling Debug x64
	devenv $solution_path /build "Debug|x64"
}
if ($documentation -eq "true") {
	Write-Host Compiling Documentation
	devenv $solution_path /build "Release|x64" /Project "Documentation"
}

### get additional version information

Write-Host Getting additional version information
# check if git is installed and accessible
[bool] $is_git_installed = $false
try {
	git | Out-Null
	$is_git_installed = $true
}
catch [System.Management.Automation.CommandNotFoundException] {
    Write-Warning "Git not found. No additional version information will be generated"
}
# check if it is a git repository
[bool] $is_git_repo = $false
if ($is_git_installed -eq $true) {
	$rev_parse_output = git rev-parse --is-inside-work-tree
	if ($rev_parse_output -eq 'true') {
		$is_git_repo = $true
	}
	else {
		Write-Warning "Not a git repository. No additional version information will be generated"
	}
}
# get name of the current git branch
$branch = ""
if ($is_git_repo -eq $true) {
	try { $branch = git rev-parse --abbrev-ref HEAD }
	catch {
		Write-Warning "Can not determine branch name. No additional version information will be generated"
	}
}

### run installer compilation
$command = '"..\Compiler\ISCC" "..\Scripts\Main.iss" "/dSolutionDir=$solution_dir" "/dQtPath=$qt_install_dir\.."'
if ($branch) {
	$command = $command + ' "/dMyAppBranch=$branch"'
}
if ($platforms -eq "Win32" -Or $platforms -eq "Both" -Or $platforms -eq "") {
	$command = $command + ' "/dIsIncludeX32=1"'
}
if ($platforms -eq "x64" -Or $platforms -eq "Both" -Or $platforms -eq "") {
	$command = $command + ' "/dIsIncludeX64=1"'
}
if ($sdk_type -eq "Sources") {
	$command = $command + ' "/dIsWithSrc=1"'
}

Invoke-Expression "& $command"
