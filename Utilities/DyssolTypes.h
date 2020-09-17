/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include <string>
#include <iostream>

// Time dependent value.
struct STDValue
{
	double time{ 0.0 };
	double value{ 0.0 };
	STDValue() {}
	STDValue(double _time, double _value) : time{ _time }, value{ _value } {}
	bool operator<(const STDValue& _other) const { return time < _other.time; }
};

struct SInterval
{
	double min;
	double max;
	friend std::ostream& operator << (std::ostream& os, const SInterval& val) { os << val.min << ' ' << val.max;	return os; }
	friend std::istream& operator >> (std::istream& is, SInterval& val) { is >> val.min >> val.max; return is; }
	bool operator==(const SInterval& _i) const { return min == _i.min && max == _i.max; }
};

enum class EArguments
{
	SOURCE_FILE,
	RESULT_FILE,
	MATERIALS_DATABASE,
	MODELS_PATH,
	SIMULATION_TIME,
	RELATIVE_TOLERANCE,
	ABSOLUTE_TOLERANCE,
	MINIMAL_FRACTION,
	INIT_TIME_WINDOW,
	MIN_TIME_WINDOW,
	MAX_TIME_WINDOW,
	MAX_ITERATIONS_NUM,
	WINDOW_CHANGE_RATE,
	ITER_UPPER_LIMIT,
	ITER_LOWER_LIMIT,
	ITER_UPPER_LIMIT_1,
	CONVERGENCE_METHOD,
	ACCEL_PARAMETER,
	RELAX_PARAMETER,
	EXTRAPOL_METHOD,
	DISTRIBUTION_GRID,
	UNIT_PARAMETER,
	UNIT_HOLDUP_MTP,
	UNIT_HOLDUP_PHASES,
	UNIT_HOLDUP_COMP,
	UNIT_HOLDUP_SOLID
};

struct SCacheSettings
{
	bool isEnabled{ false };
	size_t window{ DEFAULT_CACHE_WINDOW };
	std::wstring path{ L"" };
};

struct SPhaseDescriptor
{
	EPhase state;		// Phase state.
	std::string name;	// Phase name.
};

struct SOverallDescriptor
{
	EOverall type;
	std::string name;
	std::string units;
};