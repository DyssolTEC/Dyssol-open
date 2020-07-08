/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <map>
#include <vector>

// Represents a dependent value, described with the list of [param:value]. Implements internal linear interpolation of values between defined parameters and nearest-neighbor extrapolation beyond.
class CDependentValues
{
	std::map<double, double> m_data;

public:
	// Returns true if no [param:value] pairs have been defined.
	bool empty() const;
	// Returns number of defined pairs [param:value].
	size_t size() const;

	std::map<double, double>::iterator begin();
	std::map<double, double>::const_iterator begin() const;
	std::map<double, double>::iterator end();
	std::map<double, double>::const_iterator end() const;
	std::pair<double, double> front();
	std::pair<double, double> front() const;
	std::pair<double, double> back();
	std::pair<double, double> back() const;

	// Returns linearly interpolated value, which corresponds to a specified parameter. Performs nearest-neighbor extrapolation of data: if specified parameter lays out of the limits, returns value at the nearest limit. Returns 0, if there are no data at all.
	double GetValue(double _param) const;
	// Sets new point of [_param:_value] to the list. If the specified parameter has already been defined, overwrites its value.
	void SetValue(double _param, double _value);
	// Removes point with the given _param from the list if it exists.
	void RemoveValue(double _param);

	// Returns param by the specified index.
	double GetParamAt(size_t _index) const;
	// Returns value by the specified index.
	double GetValueAt(size_t _index) const;

	// Returns a [param:value] pair by the specified index.
	std::pair<double, double> GetPairAt(size_t _index) const;
	// Removes a [param:value] pair with the specified index.
	void RemovePairAt(size_t _index);

	// Returns list of all defined params.
	std::vector<double> GetParamsList() const;
	// Returns list of all defined values.
	std::vector<double> GetValuesList() const;

	// Returns whether the value for specified parameter has been defined in the list.
	bool IsDefined(double _param);
	// Removes all previously defined values from the list.
	void clear();

	// Output stream operator.
	friend std::ostream& operator<<(std::ostream& _os, const CDependentValues& _obj);

	// Comparison.
	bool operator==(const CDependentValues& _v) const;

private:
	// Performs linear interpolation. If the parameter is out of defined limits, performs nearest-neighbor extrapolation of data.
	double Interpolate(double _param) const;
};
