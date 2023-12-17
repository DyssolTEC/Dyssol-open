/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <map>
#include <vector>
#include <istream>
#include <limits>

/* Represents a dependent value, described with the list of [param:value].
 * Implements internal linear interpolation of values between existing parameters
 * and the nearest neighbor extrapolation for parameters outside the limits of existing parameters. */
class CDependentValues
{
	inline static const double m_eps{ 16 * std::numeric_limits<double>::epsilon() };

	std::vector<double> m_params;
	std::vector<double> m_values;

public:
	CDependentValues() = default;
	CDependentValues(const std::vector<double>& _params, const std::vector<double>& _values);

	// Returns true if no [param:value] pairs have been defined.
	[[nodiscard]] bool IsEmpty() const;
	// Returns number of defined pairs [param:value].
	[[nodiscard]] size_t Size() const;

	// Returns linearly interpolated value, which corresponds to a specified parameter. Performs nearest-neighbor extrapolation of data: if specified parameter lays out of the limits, returns value at the nearest limit. Returns 0, if there are no data at all.
	[[nodiscard]] double GetValue(double _param) const;
	// Sets new point of [_param:_value] to the list. If the specified parameter has already been defined, overwrites its value.
	void SetValue(double _param, double _value);
	// Sets new points of [_params:_values] to the list. If the specified parameter has already been defined, overwrites its value. _params and _values must have the same size.
	void SetValues(const std::vector<double>& _params, const std::vector<double>& _values);
	// Removes point with the given _param from the list if it exists.
	void RemoveValue(double _param);

	// Returns param by the specified index.
	[[nodiscard]] double GetParamAt(size_t _index) const;
	// Returns value by the specified index.
	[[nodiscard]] double GetValueAt(size_t _index) const;

	// Returns a [param:value] pair by the specified index.
	[[nodiscard]] std::pair<double, double> GetPairAt(size_t _index) const;
	// Removes a [param:value] pair with the specified index.
	void RemovePairAt(size_t _index);

	// Returns list of all defined params.
	[[nodiscard]] std::vector<double> GetParamsList() const;
	// Returns list of all defined values.
	[[nodiscard]] std::vector<double> GetValuesList() const;
	// Sets list of params.
	void SetParamsList(const std::vector<double>& _params);
	// Sets list of values.
	void SetValuesList(const std::vector<double>& _values);

	// Returns whether the value for specified parameter has been defined in the list.
	[[nodiscard]] bool HasParam(double _param) const;
	// Checks if the values are the same for all defined params.
	[[nodiscard]] bool IsConst() const;
	// Removes all previously defined values from the list.
	void Clear();

	// Output stream operator.
	friend std::ostream& operator<<(std::ostream& _s, const CDependentValues& _obj);
	// Input stream operator.
	friend std::istream& operator>>(std::istream& _s, CDependentValues& _obj);

	// Comparison.
	bool operator==(const CDependentValues& _v) const;
};
