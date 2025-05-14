# Copyright (c) 2025, DyssolTEC GmbH. 
# All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

# directory of this script
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

# set output file path
$joinedPath = Join-Path $scriptDir "..\..\Utilities\BuildVersion.h"
$outputFile = (Resolve-Path -Path $joinedPath).Path

### get build time information

# get current date and time
$now = Get-Date

# extract version components
$year = $now.ToString("yy")
$month = $now.ToString("MM")
$day = $now.ToString("dd")
$hour = $now.ToString("HH")
$minute = $now.ToString("mm")
$second = $now.ToString("ss")

# construct build version
$build_version = "$year$month$day.$hour$minute$second"

### get git information

# check if git is installed and accessible
[bool] $is_git_installed = $false
try {
	git | Out-Null
	$is_git_installed = $true
}
catch [System.Management.Automation.CommandNotFoundException] {
    Write-Warning "Git not found. Commit hash not available."
}
# check if it is a git repository
[bool] $is_git_repo = $false
if ($is_git_installed -eq $true) {
	$rev_parse_output = git rev-parse --is-inside-work-tree
	if ($rev_parse_output -eq 'true') {
		$is_git_repo = $true
	}
	else {
		Write-Warning "Not a git repository. Commit hash not available."
	}
}
# get hash of the current commit
$hash = ""
if ($is_git_repo -eq $true) {
	try { 
		$hash = git rev-parse --short HEAD 
	}
	catch {
		Write-Warning "Can not read commit hash. Commit hash not available."
	}
}
# add hash to the build version 
if ($hash) {
	$build_version = $build_version + ".$hash"
}

# Generate header file content
$content = @"
// This file was generated automatically.
// All manual changes might be reverted after the next build.
#pragma once 
constexpr char CURRENT_BUILD_VERSION[] = `"$build_version`";
"@

### write to file

Set-Content -Path $outputFile -Value $content -Encoding ASCII

Write-Host "Generated $outputFile with build version $build_version."
