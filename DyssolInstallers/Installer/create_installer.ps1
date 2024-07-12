# Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

### read arguments

$solution_dir   = $args[0]
$solution_path  = $args[1]
$qt_install_dir = $args[2]
$documentation  = $args[3]
$sdk            = $args[4]
$sdk_type       = $args[5]
$pre_build      = $args[6]
$pre_docs       = $args[7]

Write-Host "solution_dir   " $solution_dir
Write-Host "solution_path  " $solution_path
Write-Host "qt_install_dir " $qt_install_dir
Write-Host "documentation  " $documentation
Write-Host "sdk            " $sdk
Write-Host "sdk_type       " $sdk_type
Write-Host "pre_build      " $pre_build
Write-Host "pre_docs       " $pre_docs

### compile projects

if ($pre_build -eq "true" -Or $pre_build -eq "") {
	Write-Host Compiling Release x64
	devenv $solution_path /build "Release|x64"
	Write-Host Compiling Debug x64
	devenv $solution_path /build "Debug|x64"
}
if ($pre_docs -eq "true" -Or $pre_docs -eq "") {
	if ($documentation -eq "true") {
		Write-Host Compiling Documentation
		devenv $solution_path /build "Release|x64" /Project "Documentation"
	}
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
$command = '"..\Compiler\ISCC" "..\Scripts\Main.iss" "/dSolutionDir=$solution_dir" "/dQtPath=$qt_install_dir"'
if ($branch) {
	$command = $command + ' "/dMyAppBranch=$branch"'
}
if ($documentation -eq "true") {
	$command = $command + ' "/dIsDocs=1"'
}
if ($sdk -eq "true") {
	$command = $command + ' "/dIsWithSDK=1"'
}
if ($sdk_type -eq "Sources") {
	$command = $command + ' "/dIsWithSrc=1"'
}
$command = $command + ' ; $res=$?'

Invoke-Expression "& $command"

if ($res -eq $true) {
	exit 0
}
else {
	exit 1
}
