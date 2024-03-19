/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DistributionsFunctions.h"
#include "DyssolTypes.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

/**
 * Performs linear interpolation between two selected points.
 * \param _x1 First parameter.
 * \param _x2 Second parameter.
 * \param _y1 First value.
 * \param _y2 Second value.
 * \param _x Target parameter.
 * \return Interpolated value.
 */
double inline Interpolate(double _x1, double _x2, double _y1, double _y2, double _x)
{
	return (_y2 - _y1) / (_x2 - _x1) * (_x - _x1) + _y1;
}

/**
 * Interpolates the value from the input vector time-dependent _values for a given _time.
 * Assumes input vector _values is sorted in ascending order.
 * Performs linear interpolation between existing points
 * or nearest-neighbor extrapolation, if the given _time lays outside the interval of _values.
 * \param _values Sorted vector of time-dependent values.
 * \param _time Time for which interpolation is sought.
 * \param _epsilon Desired accuracy of comparisons (optional).
 * \return Interpolated value.
 */
double inline Interpolate(const std::vector<STDValue>& _values, double _time, double _epsilon = 16 * std::numeric_limits<double>::epsilon())
{
	if (_values.empty()) return {}; // no values

	const auto upper = std::upper_bound(_values.begin(), _values.end(), STDValue{ _time, {} });
	if (upper == _values.end())   return _values.back().value;  // nearest-neighbor extrapolation to the right
	if (upper == _values.begin()) return _values.front().value; // nearest-neighbor extrapolation to the left

	auto lower = upper;
	--lower;

	if (std::abs(upper->time - _time) <= _epsilon) return upper->value; // exact value found
	if (std::abs(lower->time - _time) <= _epsilon) return lower->value; // exact value found

	return Interpolate(lower->time, upper->time, lower->value, upper->value, _time); // linearly interpolated value
}

/**
 * Interpolates the Y value from the input vectors _xs and _ys for a given _x.
 * Assumes input vector _xs is sorted in ascending order. _xs and _ys must be the same size.
 * Performs linear interpolation between existing points
 * or nearest-neighbor extrapolation, if the given _x lays outside the interval of _xs.
 * \param _xs Sorted vector of parameters.
 * \param _ys Vector of values.
 * \param _x Parameter for which interpolation is sought.
 * \param _epsilon Desired accuracy of comparisons (optional).
 * \return Interpolated value.
 */
inline double Interpolate(const std::vector<double>& _xs, const std::vector<double>& _ys, double _x, double _epsilon = 16 * std::numeric_limits<double>::epsilon())
{
	if (_xs.size() != _ys.size() || _xs.empty()) return {};

	const auto upper = std::upper_bound(_xs.begin(), _xs.end(), _x);
	if (upper == _xs.end())   return _ys.back();  // nearest-neighbor extrapolation to the right
	if (upper == _xs.begin()) return _ys.front(); // nearest-neighbor extrapolation to the left

	auto lower = upper;
	--lower;

	const double upperX = *upper;
	const double lowerX = *lower;
	const double upperY = _ys[std::distance(_xs.begin(), upper)];
	const double lowerY = _ys[std::distance(_xs.begin(), lower)];

	if (std::abs(upperX - _x) <= _epsilon) return upperY; // exact value found
	if (std::abs(lowerX - _x) <= _epsilon) return lowerY; // exact value found

	return Interpolate(lowerX, upperX, lowerY, upperY, _x); // linearly interpolated value
}

// Performs spline extrapolation using three points.
double inline Extrapolate(double Y0, double Y1, double Y2, double X0, double X1, double X2, double X)
{
	struct sSplineDescr	{ double x, a, b, c, d; } splines[3];

	splines[0].x = X0; splines[1].x = X1; splines[2].x = X2;
	splines[0].a = Y0; splines[1].a = Y1; splines[2].a = Y2;
	splines[0].c = 0.;

	double alpha[2] = { 0., 0. };
	double beta[2] = { 0., 0. };

	const double A = X1 - X0;
	const double B = X2 - X1;
	const double C = 2. * (A + B);
	const double F = 6. * ((Y2 - Y1) / B - (Y1 - Y0) / A);
	const double z = (A * alpha[0] + C);
	alpha[1] = -B / z;
	beta[1] = (F - A * beta[0]) / z;

	splines[2].c = (F - A * beta[1]) / (C + A * alpha[1]);
	splines[1].c = alpha[1] * splines[2].c + beta[1];

	const double h_i = X2 - X1;
	splines[2].d = (splines[2].c - splines[1].c) / h_i;
	splines[2].b = h_i * (2. * splines[2].c + splines[1].c) / 6. + (Y2 - Y1) / h_i;

	const double dx = (X - splines[2].x);
	return splines[2].a + (splines[2].b + (splines[2].c / 2. + splines[2].d * dx / 6.) * dx) * dx;
}

/**
 * \brief Finds values laying before and next to the given value.
 * Assumes input vector is sorted in ascending order.
 * If exact value is found in the vector, it is returned. If vector contains only one value, it is returned.
 * If the value lays outside the vector, the closest outermost element of the vector is returned.
 * In general, if only one value is to be returned, it is returned in both values of the pair.
 * Additionally, one can specify the precision for comparing values.
 * \param _vector Sorted input vector of values.
 * \param _value Search value.
 * \param _epsilon Desired accuracy of comparisons (optional).
 * \return A pair of vector values lying on either side of the given value.
 */
inline std::pair<double, double> FindNeighbors(const std::vector<double>& _vector, double _value, double _epsilon = 16 * std::numeric_limits<double>::epsilon())
{
	std::pair<double, double> res{ {}, {} };

	if (_vector.empty()) return res;
	if (_vector.size() == 1) { res.first = res.second = _vector.front(); return res; }

	auto upper = std::upper_bound(_vector.begin(), _vector.end(), _value);
	if (upper == _vector.end())   { res.first = res.second = *(--upper); return res; } // value is larger than the last - use the last one
	if (upper == _vector.begin()) { res.first = res.second = *upper;     return res; } // value is smaller than the first - use the first one

	const auto lower = upper - 1;
	if (std::abs(*upper - _value) <= _epsilon) { res.first = res.second = *upper; return res; } // exact value found
	if (std::abs(*lower - _value) <= _epsilon) { res.first = res.second = *lower; return res; } // exact value found

	// two values
	res.first  = *lower;
	res.second = *upper;
	return res;
}

// Return the sum of all elements of the vector
template<typename T>
T VectorSum(const std::vector<T>& _vVec)
{
	return std::accumulate(_vVec.begin(), _vVec.end(), 0.);
}

// Normalizes provided vector.
void inline Normalize(std::vector<double>& _vec)
{
	const double sum = VectorSum(_vec);
	if (sum == 0.0 || sum == 1.0) return;
	std::transform(_vec.begin(), _vec.end(), _vec.begin(), [sum](const double v) { return v / sum; });
}

// Returns normalized copy of a vector.
std::vector<double> inline Normalized(const std::vector<double>& _vec)
{
	std::vector<double> res{ _vec };
	Normalize(res);
	return res;
}

void inline Normalize(std::vector<std::vector<double>>& _matr)
{
	double sum = 0.0;
	for (auto& row : _matr)
		sum += VectorSum(row);
	if (sum == 0.0 || sum == 1.0) return;
	for (auto& row : _matr)
		std::transform(row.begin(), row.end(), row.begin(), [sum](const double v) { return v / sum; });
}

std::vector<std::vector<double>> inline Normalize(const std::vector<std::vector<double>>& _matr)
{
	std::vector<std::vector<double>> res{ _matr };
	Normalize(res);
	return res;
}

bool inline CompareMatrices(const std::vector<std::vector<double>>& _vvMatr1, const std::vector<std::vector<double>>& _vvMatr2)
{
	if (_vvMatr1.size() != _vvMatr2.size())
		return false;

	const double dCompValue = 1e-12;
	for (size_t i = 0; i < _vvMatr1.size(); ++i)
	{
		if (_vvMatr1[i].size() != _vvMatr2[i].size())
			return false;
		for (size_t j = 0; j < _vvMatr1[i].size(); ++j)
			if (std::fabs(_vvMatr1[i][j] - _vvMatr2[i][j]) >= dCompValue)
				return false;
	}
	return true;
}

// TODO: remove it when MDMatrix is rewritten.
bool inline IncrementCoordsOld(std::vector<unsigned>& _vCoords, const std::vector<unsigned>& _vSizes)
{
	if (_vCoords.size() == 0)
		return false;

	unsigned nFactor = 0;
	if (_vCoords.size() + 1 == _vSizes.size())
		nFactor = 2;
	else if (_vCoords.size() == _vSizes.size())
		nFactor = 1;
	else
		return false;

	size_t iCoord = _vCoords.size();
	size_t iDims = _vSizes.size();
	iCoord--;
	iDims -= nFactor;

	while (true)
	{
		_vCoords[iCoord]++;
		if (_vCoords[iCoord] == _vSizes[iDims])
		{
			if (iCoord == 0)
				return false;
			_vCoords[iCoord] = 0;
			iCoord--;
			iDims--;
		}
		else
			return true;
	}
}

// Increments coordinates according to dimensions' sizes.
bool inline IncrementCoords(std::vector<size_t>& _coords, const std::vector<size_t>& _sizes)
{
	if (_coords.empty()) return false;

	unsigned corr = 0;
	if (_coords.size() + 1 == _sizes.size())
		corr = 2;
	else if (_coords.size() == _sizes.size())
		corr = 1;
	else
		return false;

	size_t iCoord = _coords.size() - 1;
	size_t iDim = _sizes.size() - corr;

	while (true)
	{
		_coords[iCoord]++;
		if (_coords[iCoord] == _sizes[iDim])
		{
			if (iCoord == 0)
				return false;
			_coords[iCoord] = 0;
			iCoord--;
			iDim--;
		}
		else
			return true;
	}
}

// Converts enumerator value to its underlying type.
template<typename E>
constexpr auto E2I(E e) -> std::underlying_type_t<E>
{
	return static_cast<std::underlying_type_t<E>>(e);
}

// Converts vector of enumerators to the vector of its underlying type.
template <typename T>
std::vector<std::underlying_type_t<T>> E2I(const std::vector<T>& _enums)
{
	if (_enums.empty()) return {};
	using integral_type = std::underlying_type_t<T>;
	std::vector<integral_type> res;
	res.reserve(_enums.size());
	for (const auto& e : _enums)
		res.push_back(static_cast<integral_type>(e));
	return res;
}

// Converts a list of enumerators to the vector of its underlying type.
template <typename T>
std::vector<std::underlying_type_t<T>> E2I(const std::initializer_list<T>& _enums)
{
	return E2I<T>(std::vector<T>{ _enums });
}

// Casts the vector of values to a vector of another type.
template <typename TO, typename TI>
std::vector<TO> vector_cast(const std::vector<TI>& _ints)
{
	if (_ints.empty()) return {};
	std::vector<TO> res;
	for (const auto& i : _ints)
		res.push_back(static_cast<TO>(i));
	return res;
}

// Converts enumerator value to double type.
template<typename E>
constexpr double E2D(E e)
{
	return static_cast<double>(static_cast<std::underlying_type_t<E>>(e));
}

// Converts double value to enumerator.
template<typename E>
constexpr E D2E(double d)
{
	return static_cast<E>(static_cast<std::underlying_type_t<E>>(d));
}

/**
 * \brief Converts numeric value to enumerator identifier.
 * \details Works for double, int, unsigned and similar types. Call it as
 * auto eval = V2E<EEnumType>(val).
 * \tparam E Type of output enum.
 * \tparam T Type of input value.
 * \param v Value.
 * \return Value converted to the enumerator identifier.
 */
template<typename E, typename T>
constexpr E V2E(T v)
{
	return static_cast<E>(static_cast<std::underlying_type_t<E>>(v));
}

/**
 * Returns normally distributed probability density function.
 * \param _grid Grid of the distribution parameter.
 * \param _d50 Mean value.
 * \param _sigma Standard deviation.
 * \return Distribution.
 */
std::vector<double> inline CreateDistributionNormal(const std::vector<double>& _grid, double _d50, double _sigma)
{
	std::vector<double> res;
	if (_grid.size() <= 1) return res;
	std::vector<double> x(_grid.size() - 1);
	for (size_t i = 0; i < _grid.size() - 1; ++i)
		x[i] = (_grid[i] + _grid[i + 1]) / 2;
	res.resize(x.size());
	const double A = 1 / (_sigma * std::sqrt(2 * MATH_PI));
	for (size_t i = 0; i < x.size(); ++i)
		res[i] = A * std::exp(-std::pow(x[i] - _d50, 2) / (2 * _sigma * _sigma));
	return Convertq3ToMassFractions(_grid, res);
}

/**
 * Returns log-normally distributed probability density function.
 * \param _grid Grid of the distribution parameter.
 * \param _d50 Mean value.
 * \param _deviation Standard deviation.
 * \return Distribution.
 */
std::vector<double> inline CreateDistributionLogNormal(const std::vector<double>& _grid, double _d50, double _deviation)
{
	std::vector<double> res;
	if (_grid.size() <= 1) return res;
	if (_deviation == 0.0) return res;
	std::vector<double> x(_grid.size() - 1);
	for (size_t i = 0; i < _grid.size() - 1; ++i)
		x[i] = (_grid[i] + _grid[i + 1]) / 2;
	res.resize(x.size());
	const double A = 1 / (_deviation * std::sqrt(2 * MATH_PI));
	for (size_t i = 0; i < x.size(); ++i)
		res[i] = A / x[i] * std::exp(-std::pow(std::log(x[i]) - std::log(_d50), 2) / (2 * _deviation * _deviation));
	return Convertq3ToMassFractions(_grid, res);
}

/**
 * Returns Rosin-Rammler-Sperling-Bennett distributed probability density function.
 * \param _grid Grid of the distribution parameter.
 * \param _d63 Scale.
 * \param _dispersion Shape.
 * \return Distribution.
 */
std::vector<double> inline CreateDistributionRRSB(const std::vector<double>& _grid, double _d63, double _dispersion)
{
	std::vector<double> res;
	if (_grid.size() <= 1) return res;
	if (_d63 == 0.0) return res;
	std::vector<double> x(_grid.size() - 1);
	for (size_t i = 0; i < _grid.size() - 1; ++i)
		x[i] = (_grid[i] + _grid[i + 1]) / 2;
	res.resize(x.size());
	for (size_t i = 0; i < x.size(); ++i)
		res[i] = 1 - std::exp(-std::pow(x[i] / _d63, _dispersion));
	return ConvertQ3ToMassFractions(res);
}

/**
 * Returns Gates-Gaudin-Schumann distributed probability density function.
 * \param _grid Grid of the distribution parameter.
 * \param _dmax Scale.
 * \param _dispersion Shape.
 * \return Distribution.
 */
std::vector<double> inline CreateDistributionGGS(const std::vector<double>& _grid, double _dmax, double _dispersion)
{
	std::vector<double> res;
	if (_grid.size() <= 1) return res;
	if (_dmax == 0.0) return res;
	std::vector<double> x(_grid.size() - 1);
	for (size_t i = 0; i < _grid.size() - 1; ++i)
		x[i] = (_grid[i] + _grid[i + 1]) / 2;
	res.resize(x.size());
	for (size_t i = 0; i < x.size(); ++i)
		if (x[i] <= _dmax)
			res[i] = std::pow(x[i] / _dmax, _dispersion);
		else
			res[i] = 1.;
	return ConvertQ3ToMassFractions(res);
}

/**
 * Returns given probability density function.
 * \param _type Distribution function.
 * \param _grid Grid of the distribution parameter.
 * \param _param1 Parameter of the distribution function.
 * \param _param2 Parameter of the distribution function.
 * \return Distribution.
 */
std::vector<double> inline CreateDistribution(EDistrFunction _type, const std::vector<double>& _grid, double _param1, double _param2)
{
	switch (_type)
	{
	case EDistrFunction::Manual:    return std::vector(_grid.size(), 0.0);
	case EDistrFunction::Normal:    return CreateDistributionNormal(_grid, _param1, _param2);
	case EDistrFunction::RRSB:      return CreateDistributionRRSB(_grid, _param1, _param2);
	case EDistrFunction::GGS:       return CreateDistributionGGS(_grid, _param1, _param2);
	case EDistrFunction::LogNormal: return CreateDistributionLogNormal(_grid, _param1, _param2);
	}
	return std::vector(_grid.size(), 0.0);
}

/**
 * \brief Converts q0 distribution to another one.
 * \details If output distributions is EPSDTypes::PSD_Number, unity density and unity total mass is assumed.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input q0 distribution.
 * \return Converted distribution.
 */
std::vector<double> inline Convertq0Distribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return Convertq0Toq3(_grid, _distr);
	case PSD_Q3:       return Convertq0ToQ3(_grid, _distr);
	case PSD_q0:       return _distr;
	case PSD_Q0:       return Convertq0ToQ0 (_grid, _distr);
	case PSD_MassFrac: return Convertq0ToMassFractions(_grid, _distr);
	case PSD_Number:   return Convertq0ToNumbers(_grid, _distr);
	case PSD_q2:       return Convertq0Toq2(_grid, _distr);
	case PSD_Q2:       return Convertq0ToQ2(_grid, _distr);
	}
	return _distr;
}

/**
 * \brief Converts q2 distribution to another one.
 * \details If output distributions is EPSDTypes::PSD_Number, unity density and unity total mass is assumed.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input q2 distribution.
 * \return Converted distribution.
 */
std::vector<double> inline Convertq2Distribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return Convertq2Toq3(_grid, _distr);
	case PSD_Q3:       return Convertq2ToQ3(_grid, _distr);
	case PSD_q0:       return Convertq2Toq0(_grid, _distr);
	case PSD_Q0:       return Convertq2ToQ0(_grid, _distr);
	case PSD_MassFrac: return Convertq2ToMassFractions(_grid, _distr);
	case PSD_Number:   return Convertq2ToNumbers(_grid, _distr);
	case PSD_q2:       return _distr;
	case PSD_Q2:       return Convertq2ToQ2(_grid, _distr);
	}
	return _distr;
}

/**
 * \brief Converts q3 distribution to another one.
 * \details If output distributions is EPSDTypes::PSD_Number, unity density and unity total mass is assumed.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input q3 distribution.
 * \return Converted distribution.
 */
std::vector<double> inline Convertq3Distribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return _distr;
	case PSD_Q3:       return Convertq3ToQ3(_grid, _distr);
	case PSD_q0:       return Convertq3Toq0(_grid, _distr);
	case PSD_Q0:       return Convertq3ToQ0(_grid, _distr);
	case PSD_MassFrac: return Convertq3ToMassFractions(_grid, _distr);
	case PSD_Number:   return Convertq3ToNumbers(_grid, _distr);
	case PSD_q2:       return Convertq3Toq2(_grid, _distr);
	case PSD_Q2:       return Convertq3ToQ2(_grid, _distr);
	}
	return _distr;
}

/**
 * \brief Converts Q0 distribution to another one.
 * \details If output distributions is EPSDTypes::PSD_Number, unity density and unity total mass is assumed.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input Q0 distribution.
 * \return Converted distribution.
 */
std::vector<double> inline ConvertQ0Distribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return ConvertQ0Toq3(_grid, _distr);
	case PSD_Q3:       return ConvertQ0ToQ3(_grid, _distr);
	case PSD_q0:       return ConvertQ0Toq0(_grid, _distr);
	case PSD_Q0:       return _distr;
	case PSD_MassFrac: return ConvertQ0ToMassFractions(_grid, _distr);
	case PSD_Number:   return ConvertQ0ToNumbers(_grid, _distr);
	case PSD_q2:       return ConvertQ0Toq2(_grid, _distr);
	case PSD_Q2:       return ConvertQ0ToQ2(_grid, _distr);
	}
	return _distr;
}

/**
 * \brief Converts Q2 distribution to another one.
 * \details If output distributions is EPSDTypes::PSD_Number, unity density and unity total mass is assumed.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input Q2 distribution.
 * \return Converted distribution.
 */
std::vector<double> inline ConvertQ2Distribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return ConvertQ2Toq3(_grid, _distr);
	case PSD_Q3:       return ConvertQ2ToQ3(_grid, _distr);
	case PSD_q0:       return ConvertQ2Toq0(_grid, _distr);
	case PSD_Q0:       return ConvertQ2ToQ0(_grid, _distr);
	case PSD_MassFrac: return ConvertQ2ToMassFractions(_grid, _distr);
	case PSD_Number:   return ConvertQ2ToNumbers(_grid, _distr);
	case PSD_q2:       return ConvertQ2Toq2(_grid, _distr);
	case PSD_Q2:       return _distr;
	}
	return _distr;
}

/**
 * \brief Converts Q3 distribution to another one.
 * \details If output distributions is EPSDTypes::PSD_Number, unity density and unity total mass is assumed.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input Q3 distribution.
 * \return Converted distribution.
 */
std::vector<double> inline ConvertQ3Distribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return ConvertQ3Toq3(_grid, _distr);
	case PSD_Q3:       return _distr;
	case PSD_q0:       return ConvertQ3Toq0(_grid, _distr);
	case PSD_Q0:       return ConvertQ3ToQ0(_grid, _distr);
	case PSD_MassFrac: return ConvertQ3ToMassFractions(_distr);
	case PSD_Number:   return ConvertQ3ToNumbers(_grid, _distr);
	case PSD_q2:       return ConvertQ3Toq2(_grid, _distr);
	case PSD_Q2:       return ConvertQ3ToQ2(_grid, _distr);
	}
	return _distr;
}

/**
 * \brief Converts mass fractions distribution to another one.
 * \details If output distributions is EPSDTypes::PSD_Number, unity density and unity total mass is assumed.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input mass fractions distribution.
 * \return Converted distribution.
 */
std::vector<double> inline ConvertMassFractionsDistribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return ConvertMassFractionsToq3(_grid, _distr);
	case PSD_Q3:       return ConvertMassFractionsToQ3(_distr);
	case PSD_q0:       return ConvertMassFractionsToq0(_grid, _distr);
	case PSD_Q0:       return ConvertMassFractionsToQ0(_grid, _distr);
	case PSD_MassFrac: return _distr;
	case PSD_Number:   return ConvertMassFractionsToNumbers(_grid, _distr);
	case PSD_q2:       return ConvertMassFractionsToq2(_grid, _distr);
	case PSD_Q2:       return ConvertMassFractionsToQ2(_grid, _distr);
	}
	return _distr;
}

/**
 * \brief Converts numbers distribution to another one.
 * \param _type Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input numbers distribution.
 * \return Converted distribution.
 */
std::vector<double> inline ConvertNumbersDistribution(EPSDTypes _type, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_type)
	{
	case PSD_q3:       return ConvertNumbersToq3(_grid, _distr);
	case PSD_Q3:       return ConvertNumbersToQ3(_grid, _distr);
	case PSD_q0:       return ConvertNumbersToq0(_grid, _distr);
	case PSD_Q0:       return ConvertNumbersToQ0(_grid, _distr);
	case PSD_MassFrac: return ConvertNumbersToMassFractions(_grid, _distr);
	case PSD_Number:   return _distr;
	case PSD_q2:       return ConvertNumbersToq2(_grid, _distr);
	case PSD_Q2:       return ConvertNumbersToQ2(_grid, _distr);
	}
	return _distr;
}

/**
 * \brief Converts distribution of one type to another one.
 * \param _typeI Input distribution type.
 * \param _typeO Output distribution type.
 * \param _grid Distribution grid.
 * \param _distr Input distribution.
 * \return Converted distribution.
 */
std::vector<double> inline ConvertDistribution(EPSDTypes _typeI, EPSDTypes _typeO, const std::vector<double>& _grid, const std::vector<double>& _distr)
{
	switch (_typeI)
	{
	case PSD_q3:       return Convertq3Distribution           (_typeO, _grid, _distr);
	case PSD_Q3:       return ConvertQ3Distribution           (_typeO, _grid, _distr);
	case PSD_q0:       return Convertq0Distribution           (_typeO, _grid, _distr);
	case PSD_Q0:       return ConvertQ0Distribution           (_typeO, _grid, _distr);
	case PSD_MassFrac: return ConvertMassFractionsDistribution(_typeO, _grid, _distr);
	case PSD_Number:   return ConvertNumbersDistribution      (_typeO, _grid, _distr);
	case PSD_q2:       return Convertq2Distribution           (_typeO, _grid, _distr);
	case PSD_Q2:       return ConvertQ2Distribution           (_typeO, _grid, _distr);
	}
	return _distr;
}

// Creates an equidistant distribution grid according to the given function and parameters.
std::vector<double> inline CreateGridEquidistant(size_t _classes, double _min, double _max)
{
	if (_max <= _min) return {};
	std::vector<double> res(_classes + 1);
	const double factor = (_max - _min) / _classes;
	for (size_t i = 0; i < _classes + 1; ++i)
		res[i] = _min + factor * i;
	return res;
}

// Creates a geometrically incrementing distribution grid according to the given function and parameters.
std::vector<double> inline CreateGridGeometricInc(size_t _classes, double _min, double _max)
{
	if (_max <= _min || _min == 0.0) return {};
	std::vector<double> res(_classes + 1);
	const double ratio = std::pow(_max / _min, 1. / _classes);
	double sum = 0;
	for (size_t i = 0; i < _classes; ++i)
		sum += std::pow(ratio, i);
	const double factor = (_max - _min) / sum;
	res[0] = _min;
	for (size_t i = 0; i < _classes; ++i)
		res[i + 1] = res[i] + std::pow(ratio, i) * factor;
	return res;
}

// Creates a geometrically decrementing distribution grid according to the given function and parameters.
std::vector<double> inline CreateGridGeometricDec(size_t _classes, double _min, double _max)
{
	if (_max <= _min || _min == 0.0) return {};
	std::vector<double> res(_classes + 1);
	const double ratio = 1. / std::pow(_max / _min, 1. / _classes);
	double sum = 0;
	for (size_t i = 0; i < _classes; ++i)
		sum += std::pow(ratio, i);
	const double factor = (_max - _min) / sum;
	res[0] = _min;
	for (size_t i = 0; i < _classes; ++i)
		res[i + 1] = res[i] + std::pow(ratio, i) * factor;
	return res;
}

// Creates a logarithmically incrementing distribution grid according to the given function and parameters.
std::vector<double> inline CreateGridLogarithmicInc(size_t _classes, double _min, double _max)
{
	if (_max <= _min || _min == 0.0) return {};
	std::vector<double> res(_classes + 1);
	const double l = 9. / _classes;
	for (size_t i = 0; i < _classes + 1; ++i)
		res[_classes - i] = _max - std::log10(1 + i * l) * (_max - _min);
	return res;
}

// Creates a logarithmically decrementing distribution grid according to the given function and parameters.
std::vector<double> inline CreateGridLogarithmicDec(size_t _classes, double _min, double _max)
{
	if (_max <= _min || _min == 0.0) return {};
	std::vector<double> res(_classes + 1);
	const double dL = 9. / _classes;
	for (size_t i = 0; i < _classes + 1; ++i)
		res[i] = _min + std::log10(1 + i * dL) * (_max - _min);
	return res;
}

/**
 * \brief Creates a distribution grid according to the given function and parameters.
 * \param _fun Grid function.
 * \param _min Minimum of the grid interval.
 * \param _max Maximum of the grid interval.
 * \param _classes Number of classes.
 * \return Generated grid.
 */
std::vector<double> inline CreateGrid(EGridFunction _fun, size_t _classes, double _min, double _max)
{
	const bool reversed = _min > _max;
	if (reversed)
		std::swap(_min, _max);

	std::vector<double> res = {};
	switch (_fun)
	{
	case EGridFunction::GRID_FUN_EQUIDISTANT:     res = CreateGridEquidistant(_classes, _min, _max);																	break;
	case EGridFunction::GRID_FUN_GEOMETRIC_S2L:   res = !reversed ? CreateGridGeometricInc(_classes, _min, _max)   : CreateGridGeometricDec(_classes, _min, _max);	break;
	case EGridFunction::GRID_FUN_GEOMETRIC_L2S:   res = !reversed ? CreateGridGeometricDec(_classes, _min, _max)   : CreateGridGeometricInc(_classes, _min, _max);	break;
	case EGridFunction::GRID_FUN_LOGARITHMIC_S2L: res = !reversed ? CreateGridLogarithmicInc(_classes, _min, _max) : CreateGridLogarithmicDec(_classes, _min, _max);	break;
	case EGridFunction::GRID_FUN_LOGARITHMIC_L2S: res = !reversed ? CreateGridLogarithmicDec(_classes, _min, _max) : CreateGridLogarithmicInc(_classes, _min, _max);	break;
	case EGridFunction::GRID_FUN_MANUAL:
	case EGridFunction::GRID_FUN_UNDEFINED:
		return std::vector<double>(_classes + 1);
	}

	if (reversed)
		std::reverse(res.begin(), res.end());

	return res;
}

/**
 * \brief Calculates mass of the sphere from its diameter and density.
 * \param _d Diameter of the sphere
 * \param _rho Density of the sphere
 * \return Sphere Mass of the sphere
 */
inline double DiameterToMass(double _d, double _rho)
{
	return MATH_PI / 6 * std::pow(_d, 3) * _rho;
}

/**
 * \brief Calculates masses of the spheres from their diameters and densities.
 * \param _d Diameters of the spheres
 * \param _rho Density of the spheres
 * \return Sphere Masses of the spheres
 */
inline std::vector<double> DiameterToMass(const std::vector<double>& _d, double _rho)
{
	std::vector<double> res(_d.size());
	for (size_t i = 0; i < _d.size(); ++i)
		res[i] = DiameterToMass(_d[i], _rho);
	return res;
}

// Calculates volume of the sphere from its diameter.
inline double DiameterToVolume(double _d)
{
	return MATH_PI / 6 * std::pow(_d, 3);
}

// Calculates volumes of the spheres from their diameters.
inline std::vector<double> DiameterToVolume(const std::vector<double>& _d)
{
	std::vector<double> res(_d.size());
	for (size_t i = 0; i < _d.size(); ++i)
		res[i] = DiameterToVolume(_d[i]);
	return res;
}

// Calculates surface of the sphere from its diameter.
inline double DiameterToSurface(double _d)
{
	return MATH_PI * std::pow(_d, 2);
}

// Calculates surfaces of the spheres from their diameters.
inline std::vector<double> DiameterToSurface(const std::vector<double>& _d)
{
	std::vector<double> res(_d.size());
	for (size_t i = 0; i < _d.size(); ++i)
		res[i] = DiameterToSurface(_d[i]);
	return res;
}

// Calculates diameter of the sphere from its volume.
inline double VolumeToDiameter(double _v)
{
	return std::pow(6 * _v / MATH_PI, 1./3.);
}

// Calculates diameters of the spheres from their volumes.
inline std::vector<double> VolumeToDiameter(const std::vector<double>& _v)
{
	std::vector<double> res(_v.size());
	for (size_t i = 0; i < _v.size(); ++i)
		res[i] = VolumeToDiameter(_v[i]);
	return res;
}

// Adds start and end values to the ends of the sorted vector, if missing, to create a closed interval.
inline void CloseInterval(std::vector<double>& _v, double _l, double _r)
{
	// no time points in the interval - return only limits
	if (_v.empty())
	{
		_v = { _l, _r };
		return;
	}

	// TODO: use global epsilon
	// add limits if necessary
	if (std::fabs(_v.front() - _l) > 16 * std::numeric_limits<double>::epsilon())
		_v.insert(_v.begin(), _l);
	if (std::fabs(_v.back() - _r) > 16 * std::numeric_limits<double>::epsilon())
		_v.push_back(_r);
}

// Adds start and end values to the ends of the sorted vector, if missing, to create a closed interval.
inline std::vector<double> CloseInterval(const std::vector<double>& _v, double _l, double _r)
{
	std::vector<double> res = _v;
	CloseInterval(res, _l, _r);
	return res;
}

/**
 * \brief Returns the values of the vector clamped from the left.
 * \details For each element, if (el < _min) el = _min.
 * \param _data Input vector of data.
 * \param _min Minimum allowed value.
 * \return Vector of clamped data.
 */
template <typename T>
std::vector<T> ClampL(const std::vector<T>& _data, T _min)
{
	std::vector res{ _data };
	std::for_each(res.begin(), res.end(), [&_min](T& v) { v = std::max(_min, v); });
	return res;
}

/**
 * \brief Returns the values of the vector clamped from the right.
 * \details For each element, if (el > _max) el = _max.
 * \param _data Input vector of data.
 * \param _max Maximum allowed value.
 * \return Vector of clamped data.
 */
template <typename T>
std::vector<T> ClampR(const std::vector<T>& _data, T _max)
{
	std::vector res{ _data };
	std::for_each(res.begin(), res.end(), [&_max](T& v) { v = std::min(_max, v); });
	return res;
}

/**
 * \brief Returns the values of the vector clamped from both sides.
 * \details For each element, if (el < _min) el = _min, if (el > _max) el = _max.
 * \param _data Input vector of data.
 * \param _min Minimum allowed value.
 * \param _max Maximum allowed value.
 * \return Vector of clamped data.
 */
template <typename T>
std::vector<T> Clamp(const std::vector<T>& _data, T _min, T _max)
{
	std::vector res{ _data };
	std::for_each(res.begin(), res.end(), [&_min, &_max](T& v) { v = std::clamp(v, _min, _max); });
	return res;
}

/**
 * \brief Returns the value clamped from the left.
 * \details if (v < _min) v = _min.
 * \param _value Input vector of data.
 * \param _min Minimum allowed value.
 * \return Clamped value.
 */
template <typename T>
T ClampL(const T& _value, T _min)
{
	return std::max(_min, _value);
}

/**
 * \brief Returns the value clamped from the right.
 * \details if (v > _max) v = _max.
 * \param _value Input value.
 * \param _max Maximum allowed value.
 * \return Clamped value.
 */
template <typename T>
T ClampR(const T& _value, T _max)
{
	return std::min(_max, _value);
}

/**
 * \brief Returns the value clamped from both sides.
 * \details if (v < _min) v = _min, if (v > _max) v = _max.
 * \param _value Input value.
 * \param _min Minimum allowed value.
 * \param _max Maximum allowed value.
 * \return Clamped value.
 */
template <typename T>
T Clamp(const T& _value, T _min, T _max)
{
	return std::clamp(_value, _min, _max);
}