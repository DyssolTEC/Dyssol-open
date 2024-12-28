/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "FileSystem.h"
#include <fcntl.h>
#ifdef _MSC_VER
#include <corecrt_io.h>
#include <direct.h>
#include "DyssolWindows.h"
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

	std::vector<std::filesystem::path> FilesList(const std::filesystem::path& _dirPath, const std::string& _filter)
	{
		std::vector<std::filesystem::path> files;
		if (!std::filesystem::exists(_dirPath)) return files;
		for (const auto& entry : std::filesystem::directory_iterator(_dirPath))
			if (entry.path().extension() == _filter)
				files.push_back(entry.path());
		return files;
	}

	bool IsWriteProtected(const std::wstring& _dirPath)
	{
#ifdef _MSC_VER
		const std::wstring testDir = _dirPath + L"/DyssolWriteProtectionTest";
#else
		const std::string testDir = Convert(_dirPath) + "/DyssolWriteProtectionTest";
#endif
		std::error_code ec;
		if (const bool created = std::filesystem::create_directory(testDir, ec); !created)
			return true;
		if (const bool exists = std::filesystem::exists(testDir, ec); !exists)
			return true;
		std::filesystem::remove_all(testDir, ec);
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
