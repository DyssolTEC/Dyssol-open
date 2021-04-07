/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "FileSystem.h"
#include <fcntl.h>
#include <iostream>
#ifdef _MSC_VER
#include <corecrt_io.h>
#include <direct.h>
#include <Windows.h>
#else
#include <locale>
#include <codecvt>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#endif

namespace FileSystem
{
	bool FileExists(const std::string& _filePath)
	{
		struct stat info {};
		return stat(_filePath.c_str(), &info) == 0;
	}

	bool DirExists(const std::string& _dirPath)
	{
		struct stat info {};
		if (stat(_dirPath.c_str(), &info) != 0)	return false;
		return (info.st_mode & S_IFDIR) != 0;
	}

	bool CreateDir(const std::string& _dirPath)
	{
#ifdef _MSC_VER
		return _mkdir(_dirPath.c_str()) == 0;
#else
		return mkdir(_dirPath.c_str(), 0777) == 0;
#endif
	}

	bool RemoveFile(const std::string& _filePath)
	{
		return remove(_filePath.c_str()) == 0;
	}

	bool RemoveDir(const std::string& _dirPath)
	{
#ifdef _MSC_VER
		std::string filePath;				// filepath
		WIN32_FIND_DATAA fileInformation;	// file information

		const std::string pattern = _dirPath + "\\*.*";						// pattern
		HANDLE hFile = FindFirstFileA(pattern.c_str(), &fileInformation);	// handle to directory
		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (fileInformation.cFileName[0] != '.')
				{
					filePath.erase();
					filePath = _dirPath + "\\" + fileInformation.cFileName;

					if (fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)                            // directory found
					{
						if (!RemoveDir(filePath)) return false;                                                 // recursively delete sub-directory
					}
					else
					{
						if (SetFileAttributesA(filePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE) return false; // change file attributes to be able to delete it
						if (DeleteFileA(filePath.c_str()) == FALSE) return false;                               // delete file
					}
				}
			} while (FindNextFileA(hFile, &fileInformation) == TRUE);                                           // for all files in current directory

			FindClose(hFile); // close file handle

			if (GetLastError() != ERROR_NO_MORE_FILES) return false;                                // check for errors
			if (SetFileAttributesA(_dirPath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE) return false; // change directory attributes to be able to delete it
			if (RemoveDirectoryA(_dirPath.c_str()) == FALSE) return false;                          // delete directory itself
		}
#else
		DIR* dir = opendir(_dirPath.c_str());
		struct dirent* entry = nullptr;
		while (entry = readdir(dir))
		{
			if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
			{
				const std::string absPath = _dirPath + "/" + entry->d_name;
				DIR* subDir = nullptr;
				if (subDir = opendir(absPath.c_str()))
				{
					closedir(subDir);
					RemoveDir(absPath);
				}
				else
				{
					FILE *file = nullptr;
					if (file = fopen(absPath.c_str(), "r"))
					{
						fclose(file);
						remove(absPath.c_str());
					}
				}
			}
		}
		remove(_dirPath.c_str());
#endif
		return true;
	}

	uint64_t FileSize(const std::string& _filePath)
	{
		struct stat info {};
		const int err = stat(_filePath.c_str(), &info);
		return err == 0 ? info.st_size : -1;
	}

	void ChangeFileSize(const std::string& _filePath, uint64_t _newSize)
	{
#ifdef _MSC_VER
		int fileHandler;
		if (_sopen_s(&fileHandler, _filePath.c_str(), _O_RDWR | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE) == 0)
		{
			_chsize_s(fileHandler, _newSize);
			_close(fileHandler);
		}
#else
		int fileHandler = open(_filePath.c_str(), O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
		if (fileHandler != -1)
		{
			int res = ftruncate(fileHandler, _newSize);
			close(fileHandler);
		}
#endif
	}

#ifdef _MSC_VER
	bool FileExists(const std::wstring& _filePath)
	{
		struct _stat64 info {};
		return _wstat64(_filePath.c_str(), &info) == 0;
	}

	bool DirExists(const std::wstring& _dirPath)
	{
		struct _stat64 info {};
		if (_wstat64(_dirPath.c_str(), &info) != 0)	return false;
		return (info.st_mode & S_IFDIR) != 0;
	}

	bool CreateDir(const std::wstring& _dirPath)
	{
		return _wmkdir(_dirPath.c_str()) == 0;
	}

	bool RemoveFile(const std::wstring& _filePath)
	{
		return _wremove(_filePath.c_str()) == 0;
	}

	bool RemoveDir(const std::wstring& _dirPath)
	{
		std::wstring filePath;				// filepath
		WIN32_FIND_DATA fileInformation;	// file information

		const std::wstring pattern = _dirPath + L"\\*.*";					// pattern
		HANDLE hFile = FindFirstFile(pattern.c_str(), &fileInformation);	// handle to directory
		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (fileInformation.cFileName[0] != '.')
				{
					filePath.erase();
					filePath = _dirPath + L"\\" + fileInformation.cFileName;

					if (fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)                           // directory found
					{
						if (!RemoveDir(filePath)) return false;                                                // recursively delete sub-directory
					}
					else
					{
						if (SetFileAttributes(filePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE) return false; // change file attributes to be able to delete it
						if (DeleteFile(filePath.c_str()) == FALSE) return false;                               // delete file
					}
				}
			} while (FindNextFile(hFile, &fileInformation) == TRUE);                                           // for all files in current directory

			FindClose(hFile); // close file handle

			if (GetLastError() != ERROR_NO_MORE_FILES) return false;                               // check for errors
			if (SetFileAttributes(_dirPath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE) return false; // change directory attributes to be able to delete it
			if (RemoveDirectory(_dirPath.c_str()) == FALSE) return false;                          // delete directory itself
		}
		return true;
	}

	uint64_t FileSize(const std::wstring& _filePath)
	{
		struct _stat64 info {};
		const int err = _wstat64(_filePath.c_str(), &info);
		return err == 0 ? info.st_size : -1;
	}

	void ChangeFileSize(const std::wstring& _filePath, uint64_t _newSize)
	{
		int fileHandler;
		if (_wsopen_s(&fileHandler, _filePath.c_str(), _O_RDWR | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE) == 0)
		{
			_chsize_s(fileHandler, _newSize);
			_close(fileHandler);
		}
	}
#endif

	std::wstring FileExtension(const std::wstring& _filePath)
	{
		const size_t slashPos = _filePath.find_last_of(L"/\\");
		const std::wstring lastWord = slashPos != std::string::npos ? _filePath.substr(slashPos + 1) : _filePath;
		const size_t dotPos = lastWord.find_last_of('.');
		return dotPos != std::string::npos ? lastWord.substr(dotPos + 1) : L"";
	}

	std::wstring FileName(const std::wstring& _filePath)
	{
		const size_t slashPos = _filePath.find_last_of(L"/\\");
		const std::wstring lastWord = slashPos != std::string::npos ? _filePath.substr(slashPos + 1) : _filePath;
		return lastWord.substr(0, lastWord.find_last_of('.'));
	}

	std::wstring FilePath(const std::wstring& _filePath)
	{
		return _filePath.substr(0, _filePath.find_last_of(L"/\\"));
	}

	std::wstring ExecutableDirPath()
	{
		std::wstring fullPath;
#ifdef _MSC_VER
		size_t copied;
		do
		{
			fullPath.resize(fullPath.size() + MAX_PATH);
			copied = GetModuleFileName(nullptr, &fullPath[0], static_cast<DWORD>(fullPath.size()));
		} while (copied >= fullPath.size());
		fullPath.resize(copied);
#else
		std::vector<char> path;
		size_t copied;
		do
		{
			path.resize(path.size() + PATH_MAX);
			copied = ::readlink("/proc/self/exe", &path[0], path.size());
			if (copied == -1) return {};
		} while (copied >= path.size());
		path.push_back('\0');
		fullPath = Convert(std::string{ path.begin(), path.end() });
#endif
		const std::wstring::size_type pos = fullPath.find_last_of(L"\\/");
		return fullPath.substr(0, pos);
	}

	std::wstring AbsolutePath(const std::wstring& _path)
	{
#ifdef _MSC_VER
		std::wstring absPath;
		size_t copied;
		do
		{
			absPath.resize(absPath.size() + MAX_PATH);
			copied = GetFullPathName(_path.c_str(), static_cast<DWORD>(absPath.size()), &absPath[0], nullptr);
		} while (copied >= absPath.size());
		absPath.resize(copied);
		return absPath;
#else
		char absPath[PATH_MAX + 1];
		char* res = realpath(Convert(_path).c_str(), absPath);
		if (!res) return {};
		return Convert(std::string{ absPath });
#endif
	}

	std::vector<std::wstring> FilesList(const std::wstring& _dirPath, const std::wstring& _filter)
	{
		std::vector<std::wstring> files;
#ifdef _MSC_VER
		const std::wstring searchPath = _dirPath + L"/" + _filter;
		WIN32_FIND_DATA fileData;
		HANDLE hFile = ::FindFirstFile(searchPath.c_str(), &fileData);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					files.push_back(_dirPath + L"/" + fileData.cFileName);
			} while (::FindNextFile(hFile, &fileData));
			::FindClose(hFile);
		}
#else
		std::vector<std::string> filesTemp;
		const std::wstring ext = FileExtension(_filter);
		DIR* dir = nullptr;
		if (dir = opendir(Convert(_dirPath).c_str()))
		{
			struct dirent* entry = nullptr;
			while (entry = readdir(dir))
			{
				if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
				if (FileExtension(Convert(entry->d_name)) == ext)
					files.push_back(_dirPath + L"/" + Convert(entry->d_name));
			}
			closedir(dir);
		}
#endif
		return files;
	}

	bool IsWriteProtected(const std::wstring& _dirPath)
	{
#ifdef _MSC_VER
		const std::wstring testDirName = _dirPath + L"/WriteProtectionTest";
#else
		const std::string testDirName = Convert(_dirPath) + "/WriteProtectionTest";
#endif
		const bool bCreated = CreateDir(testDirName);
		if (!bCreated) return true;
		const bool bExists = DirExists(testDirName);
		if (!bExists) return true;
		RemoveDir(testDirName);
		return false;
	}

#ifndef _MSC_VER
	std::string Convert(const std::wstring& _w)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(_w);
	}

	std::wstring Convert(const std::string& _s)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(_s);
	}
#endif
}
