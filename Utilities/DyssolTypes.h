/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include <string>
#include <iostream>
#include <utility>

/**
 * \private
 * \brief Time dependent value.
 */
struct STDValue
{
	double time{ 0.0 };  ///< Time
	double value{ 0.0 }; ///< Value
	/**
	 * \brief Default constructor.
	 */
	STDValue() {}
	/**
	 * \brief Constructor with initial time and value.
	 */
	STDValue(double _time, double _value) : time{ _time }, value{ _value } {}
	bool operator<(const STDValue& _other) const { return time < _other.time; }
};

/**
 * \private
 * \brief Describes an interval between two values.
 */
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

/**
 * \private
 * \brief Describes cache settings.
 */
struct SCacheSettings
{
	bool isEnabled{ false };
	size_t window{ DEFAULT_CACHE_WINDOW };
	std::wstring path{ L"" };
};

/**
 * \private
 * \brief Describes tolerance settings.
 */
struct SToleranceSettings
{
	double toleranceAbs{ DEFAULT_A_TOL };		// Absolute tolerance.
	double toleranceRel{ DEFAULT_R_TOL };		// Relative tolerance.
	double minFraction{ DEFAULT_MIN_FRACTION };	// Minimum considering fraction in MD distributions.
};

/**
 * \private
 * \brief Describes thermodynamics settings.
 */
struct SThermodynamicsSettings
{
	SInterval limits{ DEFAULT_ENTHALPY_MIN_T, DEFAULT_ENTHALPY_MAX_T };
	size_t intervals{ DEFAULT_ENTHALPY_INTERVALS };
};

/**
 * \private
 * \brief Descriptor of the phase.
 */
struct SPhaseDescriptor
{
	EPhase state;		// Phase state.
	std::string name;	// Phase name.
	bool operator==(const SPhaseDescriptor& _other) const { return state == _other.state && name == _other.name; }
};

/**
 * \private
 * \brief Descriptor of the overall property.
 */
struct SOverallDescriptor
{
	EOverall type;
	std::string name;
	std::string units;
};

/**
 * Describes a 2D point.
 */
class CPoint
{
public:
	double x = 0.0; ///< X-value.
	double y = 0.0; ///< Y-value.
	/**
	 * \brief Default constructor.
	 */
	CPoint() = default;
	/**
	 * \brief Creates a point with default value.
	 * \param _x X-value of the point.
	 * \param _y Y-value of the point.
	 */
	CPoint(double _x, double _y) : x{ _x },	y{ _y } {}
	/**
	 * \brief Number of values in the point.
	 * \return 2.
	 */
	static size_t Size() { return 2; }
	/**
	 * \brief Returns value with the given index.
	 * \details Index may be 0 or 1, otherwise std::out_of_range exception is thrown.
	 * \param _i Index.
	 * \return Target value.
	 */
	double operator[](size_t _i) const { switch (_i) { case 0: return x; case 1: return y; default: throw std::out_of_range("CPoint::operator[size_t] : index is out of range"); } }
	/**
	 * \brief Returns reference to value with the given index.
	 * \details Index may be 0 or 1, otherwise std::out_of_range exception is thrown.
	 * \param _i Index.
	 * \return Returns to target value.
	 */
	double& operator[](size_t _i) { switch (_i) { case 0: return x; case 1: return y; default: throw std::out_of_range("CPoint::operator[size_t] : index is out of range"); } }
};

enum class EDirection
{
	UP, DOWN
};

/**
 * \private
 * \brief Describes a single connection between two units in the flowsheet.
 */
struct SFlowsheetConnection
{
	std::string stream{};	// key
	std::string unitO{};	// key
	std::string portO{};	// name
	std::string unitI{};	// key
	std::string portI{};	// name
	SFlowsheetConnection(std::string _stream, std::string _unitO, std::string _portO, std::string _unitI, std::string _portI)
		: stream{ std::move(_stream) }, unitO{ std::move(_unitO) }, portO{ std::move(_portO) }, unitI{ std::move(_unitI) }, portI{ std::move(_portI) } {}
};