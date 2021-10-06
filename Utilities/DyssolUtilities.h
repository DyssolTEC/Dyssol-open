/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include "DistributionsFunctions.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

// Performs linear interpolation between two selected points.
double inline Interpolate(double Y1, double Y2, double X1, double X2, double X)
{
	return (Y2 - Y1) / (X2 - X1) * (X - X1) + Y1;
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
constexpr auto E2I(E e) -> typename std::underlying_type<E>::type
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

// Converts vector of enumerators to the vector of its underlying type.
template <typename T>
std::vector<typename std::underlying_type<T>::type> E2I(const std::vector<T>& _enums)
{
	if (_enums.empty()) return {};
	using integral_type = typename std::underlying_type<T>::type;
	std::vector<integral_type> res;
	for (const auto& e : _enums)
		res.push_back(static_cast<integral_type>(e));
	return res;
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

// Converts double value to enumerator.
template<typename E>
constexpr E D2E(double d)
{
	return static_cast<E>(static_cast<typename std::underlying_type<E>::type>(d));
}

std::vector<double> inline CreateDistributionNormal(const std::vector<double>& _x, double _d50, double _sigma)
{
	std::vector<double> vRes(_x.size());
	const double dA = 1 / (_sigma * std::sqrt(2 * MATH_PI));
	for (size_t i = 0; i < _x.size(); ++i)
		vRes[i] = dA * std::exp(-std::pow(_x[i] - _d50, 2) / (2 * _sigma * _sigma));
	return vRes;
}

std::vector<double> inline CreateDistributionLogNormal(const std::vector<double>& _x, double _d50, double _deviation)
{
	std::vector<double> vRes(_x.size());
	if (_deviation == 0) return vRes;
	const double dA = 1 / (_deviation * std::sqrt(2 * MATH_PI));
	for (size_t i = 0; i < _x.size(); ++i)
		vRes[i] = dA / _x[i] * std::exp(-std::pow((std::log(_x[i]) - std::log(_d50)), 2) / (2 * _deviation * _deviation));
	return vRes;
}

std::vector<double> inline CreateDistributionRRSB(const std::vector<double>& _x, double _d63, double _dispersion)
{
	std::vector<double> vRes(_x.size());
	if (_d63 == 0) return vRes;
	for (size_t i = 0; i < _x.size(); ++i)
		vRes[i] = 1 - std::exp(-std::pow(_x[i] / _d63, _dispersion));
	return ConvertQ3ToMassFractions(vRes);
}

std::vector<double> inline CreateDistributionGGS(const std::vector<double>& _x, double _dmax, double _dispersion)
{
	std::vector<double> vRes(_x.size());
	if (_dmax == 0) return vRes;
	for (size_t i = 0; i < _x.size(); ++i)
		if (_x[i] <= _dmax)
			vRes[i] = std::pow(_x[i] / _dmax, _dispersion);
		else
			vRes[i] = 1.;
	return ConvertQ3ToMassFractions(vRes);
}

std::vector<double> inline CreateDistribution(EDistrFunction _distr, const std::vector<double>& _x, double _param1, double _param2)
{
	switch (_distr)
	{
	case EDistrFunction::Manual:    return std::vector<double>(_x.size(), 0);
	case EDistrFunction::Normal:    return CreateDistributionNormal(_x, _param1, _param2);
	case EDistrFunction::RRSB:      return CreateDistributionRRSB(_x, _param1, _param2);
	case EDistrFunction::GGS:       return CreateDistributionGGS(_x, _param1, _param2);
	case EDistrFunction::LogNormal: return CreateDistributionLogNormal(_x, _param1, _param2);
	}
	return std::vector<double>(_x.size(), 0);
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

// Creates a distribution grid according to the given function and parameters.
std::vector<double> inline CreateGrid(EGridFunction _fun, size_t _classes, double _min, double _max)
{
	switch (_fun)
	{
	case EGridFunction::GRID_FUN_EQUIDISTANT:		return CreateGridEquidistant(_classes, _min, _max);
	case EGridFunction::GRID_FUN_GEOMETRIC_S2L:		return CreateGridGeometricInc(_classes, _min, _max);
	case EGridFunction::GRID_FUN_GEOMETRIC_L2S:		return CreateGridGeometricDec(_classes, _min, _max);
	case EGridFunction::GRID_FUN_LOGARITHMIC_S2L:	return CreateGridLogarithmicInc(_classes, _min, _max);
	case EGridFunction::GRID_FUN_LOGARITHMIC_L2S:	return CreateGridLogarithmicDec(_classes, _min, _max);
	case EGridFunction::GRID_FUN_MANUAL:
	case EGridFunction::GRID_FUN_UNDEFINED:
		return std::vector<double>(_classes + 1);
	}
	return {};
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