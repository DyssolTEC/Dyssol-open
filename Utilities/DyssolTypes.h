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
	[[nodiscard]] bool Includes(double _d) const { return _d >= min && _d <= max; }
};

// TODO: remove
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
	UNIT_HOLDUP_SOLID,
	TEXT_EXPORT_FILE,
	TEXT_EXPORT_PRECISION,
	TEXT_EXPORT_FIXED_POINT,
	TEXT_EXPORT_SIGNIFICANCE_LIMIT,
	EXPORT_MASS,
	EXPORT_PSD,
	EXPORT_GRAPH,
};

struct SCacheSettings
{
	bool isEnabled{ false };
	size_t window{ DEFAULT_CACHE_WINDOW };
	std::wstring path{ L"" };
};

struct SToleranceSettings
{
	double toleranceAbs{ DEFAULT_A_TOL };		// Absolute tolerance.
	double toleranceRel{ DEFAULT_R_TOL };		// Relative tolerance.
	double minFraction{ DEFAULT_MIN_FRACTION };	// Minimum considering fraction in MD distributions.
};

struct SThermodynamicsSettings
{
	SInterval limits{ DEFAULT_ENTHALPY_MIN_T, DEFAULT_ENTHALPY_MAX_T };
	size_t intervals{ DEFAULT_ENTHALPY_INTERVALS };
};

struct SPhaseDescriptor
{
	EPhase state;		// Phase state.
	std::string name;	// Phase name.
	bool operator==(const SPhaseDescriptor& _other) const { return state == _other.state && name == _other.name; }
};

struct SOverallDescriptor
{
	EOverall type;
	std::string name;
	std::string units;
};

// Describes a 2D point.
class CPoint
{
public:
	double x{};
	double y{};
	CPoint() = default;
	CPoint(double _x, double _y) : x{ _x },	y{ _y } {}
	static size_t Size() { return 2; }
	double operator[](size_t _i) const { switch (_i) { case 0: return x; case 1: return y; default: throw std::out_of_range("CPoint::operator[size_t] : index is out of range"); } }
	double& operator[](size_t _i) { switch (_i) { case 0: return x; case 1: return y; default: throw std::out_of_range("CPoint::operator[size_t] : index is out of range"); } }
};

enum class EDirection
{
	UP, DOWN
};

/*
 * Describes a single connection between two units in the flowsheet.
 */
struct SFlowsheetConnection
{
	std::string stream{}, unitO{}, portO{}, unitI{}, portI{};
	SFlowsheetConnection(const std::string& _stream, const std::string& _unitO, const std::string& _portO, const std::string& _unitI, const std::string& _portI)
		: stream{ _stream }, unitO{ _unitO }, portO{ _portO }, unitI{ _unitI }, portI{ _portI } {}
};