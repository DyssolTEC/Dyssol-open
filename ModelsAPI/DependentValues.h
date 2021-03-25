/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <ostream>
#include <map>
#include <vector>
#include <ostream>

/* Represents a dependent value, described with the list of [param:value].
 * Implements internal linear interpolation of values between existing parameters
 * and the nearest neighbor extrapolation for parameters outside the limits of existing parameters. */
class CDependentValues
{
public:
	using iterator = std::map<double, double>::iterator;
	using const_iterator = std::map<double, double>::const_iterator;
	using value_type = std::map<double, double>::value_type;

private:
	std::map<double, double> m_data;

public:
	// Returns true if no [param:value] pairs have been defined.
	[[nodiscard]] bool IsEmpty() const;
	// Returns number of defined pairs [param:value].
	[[nodiscard]] size_t Size() const;

	iterator begin();
	[[nodiscard]] const_iterator begin() const;
	[[nodiscard]] const_iterator cbegin() const;
	iterator end();
	[[nodiscard]] const_iterator end() const;
	[[nodiscard]] const_iterator cend() const;
	value_type front();
	[[nodiscard]] value_type front() const;
	value_type back();
	[[nodiscard]] value_type back() const;

	// Returns linearly interpolated value, which corresponds to a specified parameter. Performs nearest-neighbor extrapolation of data: if specified parameter lays out of the limits, returns value at the nearest limit. Returns 0, if there are no data at all.
	[[nodiscard]] double GetValue(double _param) const;
	// Sets new point of [_param:_value] to the list. If the specified parameter has already been defined, overwrites its value.
	void SetValue(double _param, double _value);
	// Removes point with the given _param from the list if it exists.
	void RemoveValue(double _param);

	// Returns param by the specified index.
	[[nodiscard]] double GetParamAt(size_t _index) const;
	// Returns value by the specified index.
	[[nodiscard]] double GetValueAt(size_t _index) const;

	// Returns a [param:value] pair by the specified index.
	[[nodiscard]] value_type GetPairAt(size_t _index) const;
	// Removes a [param:value] pair with the specified index.
	void RemovePairAt(size_t _index);

	// Returns list of all defined params.
	[[nodiscard]] std::vector<double> GetParamsList() const;
	// Returns list of all defined values.
	[[nodiscard]] std::vector<double> GetValuesList() const;

	// Returns whether the value for specified parameter has been defined in the list.
	[[nodiscard]] bool HasParam(double _param) const;
	// Checks if the values are the same for all defined params.
	[[nodiscard]] bool IsConst() const;
	// Removes all previously defined values from the list.
	void Clear();

	// Returns a table with only unique entries.
	static CDependentValues Unique(const CDependentValues& _table);

	// Output stream operator.
	friend std::ostream& operator<<(std::ostream& _os, const CDependentValues& _obj);

	// Comparison.
	bool operator==(const CDependentValues& _v) const;

private:
	// Performs linear interpolation. If the parameter is out of defined limits, performs nearest-neighbor extrapolation of data.
	[[nodiscard]] double Interpolate(double _param) const;
};
