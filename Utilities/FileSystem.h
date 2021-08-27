/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <string>
#include <vector>
#include <filesystem>

#ifdef AddPort
#undef AddPort
#endif

/* There are two versions of file-specific functions, since on Linux all symbols can be safely represented with std::string,
 * while on windows it required to use std::wstring.
 * Windows-specific versions, working with std::wstring (2) can be safely called on Windows only.
 * OS independent versions, working with std::string, (1) can be safely called on Windows and Linux.
 * Proper version will be selected automatically depending on the current OS,
 * if the function parameter is wrapped in a function call UnicodePath(). E.g.:
 * bool res = FileSystem::FileExists(UnicodePath(wstring_path));
 * I.e. wstring_path will be converted if needed before the call to FileSystem.
 * OS independent versions, working with std::wstring, (3) contain both versions for Windows and Linux.
 * Service functions to convert std::string <-> std::wstring are needed on Linux for conversions in (3).
 *  */
namespace FileSystem
{
	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. OS independent versions, working with std::string. */



	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Windows-specific versions, working with std::wstring and using Windows API. */


	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. OS independent versions, working with std::wstring. */

	std::wstring FileExtension(const std::wstring& _filePath);		// Returns file extension.
	std::wstring FileName(const std::wstring& _filePath);			// Returns file name without path and extension.
	std::wstring FilePath(const std::wstring& _filePath);			// Returns file path without file name and extension.

	std::wstring ExecutableDirPath();						// Returns the directory that contains the application executable.
	std::wstring AbsolutePath(const std::wstring& _path);	// Converts provided path to canonical absolute path.

	std::vector<std::filesystem::path> FilesList(const std::wstring& _dirPath, const std::wstring& _filter); // Returns the list of files with selected filter (extension), located in the specified directory.

	bool IsWriteProtected(const std::wstring& _dirPath); // Checks whether the specified path is write-protected.

	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Service functions to convert std::string <-> std::wstring. Needed for Linux. */

#ifndef _MSC_VER
	std::string Convert(const std::wstring& _w);
	std::wstring Convert(const std::string& _s);
#endif
}
