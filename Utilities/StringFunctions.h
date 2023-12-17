/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <string>
#include <vector>
#include <set>

namespace StringFunctions
{
	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Conversions. */

	std::wstring String2WString(const std::string& _s);				// Converts std::string to std::wstring using current locate.
	std::string WString2String(const std::wstring& _s);				// Converts std::wstring to std::string using current locate.

	std::string Double2String(double _v, size_t _precision = -1);	// Converts double to std::string with the specified precision.

	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Block of these functions is needed for proper work with some functions from FileSystem. See the description in FileSystem for details. */

#ifdef _MSC_VER
	std::wstring UnicodePath(const std::wstring& _path);	// Returns the string itself.
	std::wstring UnicodePath(const std::string& _path);		// Applies String2WString().
#else
	std::string UnicodePath(const std::string& _path);		// Returns the string itself.
	std::string UnicodePath(const std::wstring& _path);		// Applies WString2String().
#endif


	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Strings utilities. */

	bool CompareCaseInsensitive(const std::string& _s1, const std::string& _s2);	// Case insensitive strings comparison. Returns true if the stings are the same.
	bool CompareCaseInsensitive(const std::wstring& _s1, const std::wstring& _s2);	// Case insensitive wide strings comparison. Returns true if the stings are the same.
	bool IsDigits(const std::string& _s);											// Returns true if all symbols in the string are digits.
	bool IsSimpleUInt(const std::string& _s);										// Checks if the string represents an unsigned int value.

	bool Contains(const std::string& _s, char _c);									// Checks if the string contains a symbol.
	bool Contains(const std::string& _s, const std::string& _subs);					// Checks if the string contains a substring.
	bool StartsWith(const std::string& _s, const std::string& _subs);				// Checks if the string starts with a substring.

	void ReplaceAll(std::string& _s, const std::string& _old, const std::string& _new);					 // Replaces all occurrences of _old with _new in _s.
	void ReplaceAll(std::wstring& _s, const std::wstring& _old, const std::wstring& _new);				 // Replaces all occurrences of _old with _new in _s.
	std::string ReplaceAll(const std::string& _s, const std::string& _old, const std::string& _new);	 // Returns a copy of string _s, where all occurrences of _old are replaced with _new.
	std::wstring ReplaceAll(const std::wstring& _s, const std::wstring& _old, const std::wstring& _new); // Returns a copy of string _s, where all occurrences of _old are replaced with _new.
	std::vector<std::string> SplitString(const std::string& _s, char _delim);							 // Splits the string according to delimiter.

	void TrimWhitespaces(std::string& _s);												// Removes whitespaces from both sides of the string.
	std::string TrimWhitespaces(const std::string& _s);									// Returns a copy of string _s without whitespaces from both sides.
	void TrimFromSymbols(std::string& _s, const std::string& _symbols);					// Removes the end of the string starting with _symbols.
	std::string TrimFromSymbols(const std::string& _s, const std::string& _symbols);	// Returns a copy of string _s removing its end starting with _symbols.
	std::string RemoveQuotes(const std::string& _s);									// Returns a copy of string _s removing leading and trailing single or double quotes, if any.

	void Quote(std::string& _s);					// Adds double quotes to the string _s.
	std::string Quote(const std::string& _s);		// Returns a copy of the string _s in double quotes.
	std::string ToLowerCase(const std::string& _s);	// Returns a copy of the string _s with all characters in lower case.
	std::string ToUpperCase(const std::string& _s); // Returns a copy of the string _s with all characters in upper case.

	std::string GetRestOfLine(std::istream& _is);												// Returns the full string until the end-of-line without trailing whitespaces.
	template<typename T> T GetValueFromStream(std::istream& _is) { T v{}; _is >> v; return v; }	// Returns the next value from the stream and advances stream's iterator correspondingly.
	template<> bool GetValueFromStream(std::istream& _is);										// Returns the next value from the stream and advances stream's iterator correspondingly. Overload for bool type.
	template<> double GetValueFromStream(std::istream& _is);									// Returns the next value from the stream and advances stream's iterator correspondingly. Overload for double type.
	template<> std::string GetValueFromStream(std::istream& _is);								// Returns the next value from the stream and advances stream's iterator correspondingly. Overload for string type.
	template<> std::vector<double> GetValueFromStream(std::istream& _is);						// Returns the next value from the stream and advances stream's iterator correspondingly. Overload for vector<double>.
	template<> std::vector<uint64_t> GetValueFromStream(std::istream& _is);						// Returns the next value from the stream and advances stream's iterator correspondingly. Overload for vector<uint64_t>.
	template<> std::vector<std::string> GetValueFromStream(std::istream& _is);					// Returns the next value from the stream and advances stream's iterator correspondingly. Overload for vector<string>.
	template<typename T>
	std::enable_if_t<std::is_enum_v<T>, T> GetEnumFromStream(std::istream& _is)					// Returns the next value from the stream and advances stream's iterator correspondingly. Version for enum types.
	{ return static_cast<T>(GetValueFromStream<std::underlying_type_t<T>>(_is)); }

	std::string ToString(const std::set<double>& _set); // Returns a comma separated string representation of the set.

	std::string UnifyPath(const std::string& _path);   // Brings the path to a unified view (slashes, backslashes).
	std::wstring UnifyPath(const std::wstring& _path); // Brings the path to a unified view (slashes, backslashes).

	/*////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Strings generators. */

	std::string GenerateRandomKey(size_t _length = 20);	// Returns a random string with the specified length.

	// Returns a string with the specified length, which will be unique among the set of existing strings.
	std::string GenerateUniqueKey(const std::vector<std::string>& _existing, size_t _length = 20);
	// Returns a string with the specified length, which will be unique among the set of existing strings.
	std::string GenerateUniqueKey(const std::string& _init, const std::vector<std::string>& _existing, size_t _length = 20);
	// Returns a name consisting of _namingBase + number not yet in _existing
	std::string GenerateUniqueName(const std::string& _namingBase, const std::vector<std::string>& _existing);
}
