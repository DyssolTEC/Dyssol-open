/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StringFunctions.h"
#ifdef _MSC_VER
#include <Windows.h>
#undef AddPort
#undef CreateFile
#else
#include <codecvt>
#endif
#include <sstream>
#include <algorithm>
#include <locale>
#include <cctype>
#include <iomanip>
#include <random>

namespace StringFunctions
{
	std::wstring String2WString(const std::string& _s)
	{
		if (_s.empty()) return {};
#ifdef _MSC_VER
		const int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, _s.c_str(), static_cast<int>(_s.size()), nullptr, 0);
		if (sizeNeeded <= 0) return {};
		std::wstring res(sizeNeeded, 0);
		if (MultiByteToWideChar(CP_ACP, 0, _s.c_str(), static_cast<int>(_s.size()), &res[0], sizeNeeded) <= 0) return {};
		return res;
#else
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(_s);
#endif
	}

	std::string WString2String(const std::wstring& _s)
	{
		if (_s.empty()) return {};
#ifdef _MSC_VER
		const int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, _s.c_str(), static_cast<int>(_s.size()), nullptr, 0, nullptr, nullptr);
		if (sizeNeeded <= 0) return {};
		std::string res(sizeNeeded, 0);
		if (WideCharToMultiByte(CP_ACP, 0, _s.c_str(), static_cast<int>(_s.size()), &res[0], sizeNeeded, nullptr, nullptr) <= 0) return {};
		return res;
#else
		return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(_s);
#endif
	}

	std::string Double2String(double _v, size_t _precision)
	{
		std::ostringstream os;
		if (_precision != static_cast<size_t>(-1))
			os << setiosflags(std::ios::fixed) << std::setprecision(_precision);
		os << _v;
		return os.str();
	}

#ifdef _MSC_VER
	std::wstring UnicodePath(const std::wstring& _path)
	{
		return _path;
	}
	std::wstring UnicodePath(const std::string& _path)
	{
		return String2WString(_path);
	}
#else
	std::string UnicodePath(const std::string& _path)
	{
		return _path;
	}
	std::string UnicodePath(const std::wstring& _path)
	{
		return WString2String(_path);
	}
#endif

	bool CompareCaseInsensitive(const std::string& _s1, const std::string& _s2)
	{
		return _s1.size() == _s2.size() &&
			std::equal(_s1.begin(), _s1.end(), _s2.begin(),	[](unsigned char c1, unsigned char c2)
		{	return c1 == c2 || std::toupper(c1) == std::toupper(c2); });
	}

	bool CompareCaseInsensitive(const std::wstring& _s1, const std::wstring& _s2)
	{
		return _s1.size() == _s2.size() &&
			std::equal(_s1.begin(), _s1.end(), _s2.begin(),	[](wchar_t c1, wchar_t c2)
		{	return (c1 == c2 || std::toupper(c1) == std::toupper(c2)); });
	}

	bool IsDigits(const std::string& _s)
	{
		for (const auto c : _s)
			if (!std::isdigit(c))
				return false;
		return true;
	}

	void ReplaceAll(std::string& _s, const std::string& _old, const std::string& _new)
	{
		if (_old.empty() || _s.empty()) return;
		size_t pos = _s.find(_old);
		while (pos != std::string::npos)
		{
			_s.replace(pos, _old.length(), _new);
			pos = _s.find(_old, pos + _new.length());
		}
	}

	void ReplaceAll(std::wstring& _s, const std::wstring& _old, const std::wstring& _new)
	{
		if (_old.empty() || _s.empty()) return;
		size_t pos = _s.find(_old);
		while (pos != std::string::npos)
		{
			_s.replace(pos, _old.length(), _new);
			pos = _s.find(_old, pos + _new.length());
		}
	}

	std::string ReplaceAll(const std::string& _s, const std::string& _old, const std::string& _new)
	{
		std::string res = _s;
		ReplaceAll(res, _old, _new);
		return res;
	}

	std::wstring ReplaceAll(const std::wstring& _s, const std::wstring& _old, const std::wstring& _new)
	{
		std::wstring res = _s;
		ReplaceAll(res, _old, _new);
		return res;
	}

	std::vector<std::string> SplitString(const std::string& _s, char _delim)
	{
		std::vector<std::string> res;
		std::stringstream ss(_s);
		std::string line;
		while (std::getline(ss, line, _delim))
			res.push_back(std::move(line));
		return res;
	}

	void TrimWhitespaces(std::string& _s)
	{
		_s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(), [](char ch) { return !std::isspace(ch, std::locale::classic()); }));
		_s.erase(std::find_if(_s.rbegin(), _s.rend(), [](char ch) { return !std::isspace(ch, std::locale::classic()); }).base(), _s.end());
	}

	std::string TrimWhitespaces(const std::string& _s)
	{
		std::string res = _s;
		TrimWhitespaces(res);
		return res;
	}

	void TrimFromSymbols(std::string& _s, const std::string& _symbols)
	{
		if (_s.empty()) return;
		const std::size_t found = _s.rfind(_symbols);
		if (found != std::string::npos)	// found
			_s.erase(found, _s.size());
		return TrimWhitespaces(_s);
	}

	std::string TrimFromSymbols(const std::string& _s, const std::string& _symbols)
	{
		std::string res = _s;
		TrimFromSymbols(res, _symbols);
		return res;
	}

	void Quote(std::string& _s)
	{
		_s = "\"" + _s + "\"";
	}

	std::string Quote(const std::string& _s)
	{
		std::ostringstream ss;
		ss << std::quoted(_s);
		return ss.str();
	}

	std::string GetRestOfLine(std::istream* _is)
	{
		const std::istreambuf_iterator<char> eos;
		std::string res(std::istreambuf_iterator<char>(*_is), eos);
		TrimWhitespaces(res);
		return res;
	}

	std::string UnifyPath(const std::string& _path)
	{
		std::string s = _path;
		ReplaceAll(s, "\\", "/");
		ReplaceAll(s, "//", "/");
		return s;
	}

	std::wstring UnifyPath(const std::wstring& _path)
	{
		std::wstring s = _path;
		ReplaceAll(s, L"\\", L"/");
		ReplaceAll(s, L"//", L"/");
		return s;
	}

	// Generates a random key of the given length.
	std::string GenerateRandomKey(size_t _length /*= 20*/)
	{
		static const std::string symbols = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		std::random_device device;
		std::default_random_engine engine{ device() };
		const std::uniform_int_distribution<size_t> distribution{ 0, symbols.length() - 1 };
		std::string result(_length, '0');
		std::generate(result.begin(), result.end(), [&]() { return symbols[distribution(engine)]; });
		return result;
	}

	// Returns a key that does not yet exist in _existing.
	std::string GenerateUniqueKey(const std::vector<std::string>& _existing, size_t _length /*= 20*/)
	{
		while (true)
		{
			std::string res = GenerateRandomKey(_length);
			if (std::find(_existing.begin(), _existing.end(), res) == _existing.end())
				return res;
		}
	}

	// Returns a key that does not yet exist in _existing.
	std::string GenerateUniqueKey(const std::string& _init, const std::vector<std::string>& _existing, size_t _length /*= 20*/)
	{
		if (std::find(_existing.begin(), _existing.end(), _init) == _existing.end())
			return _init;
		return GenerateUniqueKey(_existing, _length);
	}
}
