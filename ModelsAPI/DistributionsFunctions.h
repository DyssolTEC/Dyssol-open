/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <vector>

/**
 * \file
 * \brief Several global functions are defined to work with particle size distributions. These functions can be called from any place of the code.
 * \details All functions receive grid as the input parameter. The grid can be previously obtained with the help of the function CBaseUnit::GetNumericGrid(EDistrTypes) const.
 * \note
 * - \f$d_i\f$ - diameter of particle in class \f$i\f$
 * - \f$\Delta d_i\f$ - size of the class \f$i\f$
 * - \f$M_k\f$ - \f$k\f$-th moment
 * - \f$q\f$ - density distribution
 * - \f$q_0\f$ - number related density distribution
 * - \f$Q_0\f$ - number related cumulative distribution
 * - \f$q_2\f$ - surface-area-related density distribution
 * - \f$Q_2\f$ - surface-area-related cumulative distribution
 * - \f$q_3\f$ - mass-related density distribution
 * - \f$Q_3\f$ - mass-related cumulative distribution
 * - \f$w_i\f$ - mass fraction of particles of class \f$i\f$
 * - \f$N_i\f$ - number of particles of class \f$i\f$
 * - \f$N_{tot}\f$ - total number of particles
 */

/**
 * \brief Calculates moment of the density distribution.
 * \details \f$M_k = \sum_i d_i^k q_i \Delta d_i\f$.
 * \param _moment Value of moment.
 * \param _grid Distribution grid.
 * \param _q Input distribution.
 * \return Value of the density distribution moment.
 */
double inline GetMMoment(int _moment, const std::vector<double>& _grid, const std::vector<double>& _q)
{
	if (_grid.size() != _q.size() + 1)	return 0;

	double dMoment = 0;
	for (size_t i = 0; i < _q.size(); ++i)
		dMoment += std::pow((_grid[i] + _grid[i + 1]) / 2, _moment) * _q[i] * (_grid[i + 1] - _grid[i]);
	return dMoment;
}

/**
 * \brief Calculates Q.
 * \details
 * \param _QiDistr Input distribution.
 * \param _grid Distribution grid.
 * \param _size Value of size.
 * \return Calculated Q.
 */
double inline GetQ(const std::vector<double>& _QiDistr, const std::vector<double>& _grid, double _size)
{
	if (_size <= _grid.front()) return 0;
	if (_size >= _grid.back()) return 1;

	std::vector<double> vTempDistr;
	vTempDistr.push_back(0);
	for (double v : _QiDistr)
		vTempDistr.push_back(v);

	// find indexes where interpolation should be done
	int nLeft = static_cast<int>(_grid.size()) - 1;
	int nRight = 0;
	while (nRight < static_cast<int>(_grid.size()))
		if (_grid[nRight] >= _size)
		{
			nLeft = nRight - 1;
			break;
		}
		else
			nRight++;
	if (nLeft >= 0 && _grid[nLeft] == _size)
		return vTempDistr[nLeft];
	else if (nRight < static_cast<int>(_grid.size()) && _grid[nRight] == _size)
		return vTempDistr[nRight];
	else // point inside - interpolation
		return(vTempDistr[nRight] - vTempDistr[nLeft]) / (_grid[nRight] - _grid[nLeft]) * (_size - _grid[nLeft]) + vTempDistr[nLeft];
}

/**
 * \brief Performs conversion from cumulative to density distributions.
 * \details \f$q_0 = \frac{Q_0}{\Delta d_i}\f$ and \f$q_i = \frac{Q_i - Q_{i-1}}{\Delta d_i}\f$.
 * \param _grid Distribution grid.
 * \param _Q Input distribution.
 * \return Density distribution.
 */
std::vector<double> inline Q2q(const std::vector<double>& _grid, const std::vector<double>& _Q)
{
	if (_Q.empty()) return {};
	std::vector<double> q(_Q.size());
	q[0] = _Q[0] / (_grid[1] - _grid[0]);
	for (size_t i = 1; i < _Q.size(); ++i)
		q[i] = (_Q[i] - _Q[i - 1]) / (_grid[i + 1] - _grid[i]);
	return q;
}

/**
 * \brief Performs conversion from density to cumulative distributions.
 * \details \f$Q_i = \sum_i q_i \Delta d_i = Q_i-1 + q_i \Delta d_i\f$.
 * \param _grid Distribution grid.
 * \param _q Input distribution.
 * \return Cumulative distribution.
 */
std::vector<double> inline q2Q(const std::vector<double>& _grid, const std::vector<double>& _q)
{
	std::vector<double> Q(_q.size());
	double dQ3 = 0;
	for (size_t i = 0; i < _q.size(); ++i)
	{
		dQ3 += (_grid[i + 1] - _grid[i]) * _q[i];
		Q[i] = dQ3;
	}
	return Q;
}

/**
 * \brief Performs conversion from one density distribution to another.
 * \details \f$q_i = \frac{d_i^{y-x} q_i}{M^{y-x}(q_x)}\f$.
 * \param _grid Distribution grid.
 * \param _qx Input distribution.
 * \param _x Input density distribution.
 * \param _y Output density distribution.
 * \return Density distribution.
 */
std::vector<double> inline qx2qy(const std::vector<double>& _grid, const std::vector<double>& _qx, int _x, int _y)
{
	if (_grid.size() != _qx.size() + 1) return {};

	const double M = GetMMoment(_y - _x, _grid, _qx);
	if (M == 0) return _qx;
	std::vector<double> qy(_qx.size());
	for (size_t i = 0; i < _qx.size(); ++i)
		qy[i] = std::pow((_grid[i] + _grid[i + 1]) / 2, _y - _x) * _qx[i] / M;
	return qy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Performs conversion from Q0 to q0 distributions.
 * \details Using information about the size grid: \f$q_{0,0} = \frac{Q_{0,0}}{\Delta d_i}\f$ and \f$q_{0,i} = \frac{Q_{0,i} - Q_{0,i-1}}{\Delta d_i}\f$.
 * Refer to function Q2q(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Converted q0 distribution.
 */
std::vector<double> inline ConvertQ0Toq0(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return Q2q(_grid, _Q0);
}

/**
 * \brief Performs conversion from q0 to Q0 distributions.
 * \details Using information about the size grid: \f$Q_{0,i} = \sum_i q_{0,i} \Delta d_i = Q_{0,i-1} + q_{0,i} \Delta d_i\f$.
 * Refer to function q2Q(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q0 Input distribution.
 * \return Converted Q0 distribution.
 */
std::vector<double> inline Convertq0ToQ0(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return q2Q(_grid, _q0);
}

/**
 * \brief Performs conversion from Q2 to q2 distributions.
 * \details Using information about the size grid: \f$q_{2,0} = \frac{Q_{2,0}}{\Delta d_i}\f$ and \f$q_{2,i} = \frac{Q_{2,i} - Q_{2,i-1}}{\Delta d_i}\f$.
 * Refer to function Q2q(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q2 Input distribution.
 * \return Converted q2 distribution.
 */
std::vector<double> inline ConvertQ2Toq2(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return Q2q(_grid, _Q2);
}

/**
 * \brief Performs conversion from q2 to Q2 distributions.
 * \details Using information about the size grid: \f$Q_{2,i} = \sum_i q_{2,i} \Delta d_i = Q_{2,i-1} + q_{2,i} \Delta d_i\f$.
 * Refer to function q2Q(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q2 Input distribution.
 * \return Converted Q2 distribution.
 */
std::vector<double> inline Convertq2ToQ2(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return q2Q(_grid, _q2);
}

/**
 * \brief Performs conversion from Q3 to q3 distributions.
 * \details Using information about the size grid: \f$q_{3,0} = \dfrac{Q_{3,0}}{\Delta d_i}\f$ and \f$q_{3,i} = \frac{Q_{3,i} - Q_{3,i-1}}{\Delta d_i}\f$.
 * Refer to function Q2q(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q3 Input distribution.
 * \return Converted q3 distribution.
 */
std::vector<double> inline ConvertQ3Toq3(const std::vector<double>& _grid, const std::vector<double>& _Q3)
{
	return Q2q(_grid, _Q3);
}

/**
 * \brief Performs conversion from q3 to Q3 distributions.
 * \details Using information about the size grid: \f$Q_{3,i} = \sum_i q_{3,i} \Delta d_i = Q_{3,i-1} + q_{3,i} \Delta d_i\f$.
 * Refer to function q2Q(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Converted Q3 distribution.
 */
std::vector<double> inline Convertq3ToQ3(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return q2Q(_grid, _q3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Performs conversion from q0 to q2 distributions.
 * \details Using information about the size grid by \f$q_{2,i} = \frac{d_i^2 q_{0,i}}{M_2(q_0)}\f$.
 * Refer to function qx2qy(const std::vector<double>&, const std::vector<double>&, int, int).
 * \param _grid Distribution grid.
 * \param _q0 Input distribution.
 * \return Converted q2 distribution.
 */
std::vector<double> inline Convertq0Toq2(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return qx2qy(_grid, _q0, 0, 2);
}

/**
 * \brief Performs conversion from q0 to q3 distributions.
 * \details Using information about the size grid by \f$q_{3,i} = \frac{d_i^3 q_{0,i}}{M_3(q_0)}\f$.
 * Refer to function qx2qy(const std::vector<double>&, const std::vector<double>&, int, int).
 * \param _grid Distribution grid.
 * \param _q0 Input distribution.
 * \return Converted q3 distribution.
 */
std::vector<double> inline Convertq0Toq3(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return qx2qy(_grid, _q0, 0, 3);
}

/**
 * \brief Performs conversion from q2 to q0 distributions.
 * \details Using information about the size grid by \f$q_{0,i} = \frac{d_i^{-2} q_{2,i}}{M_{-2}(q_2)}\f$.
 * Refer to function qx2qy(const std::vector<double>&, const std::vector<double>&, int, int).
 * \param _grid Distribution grid.
 * \param _q2 Input distribution.
 * \return Converted q0 distribution.
 */
std::vector<double> inline Convertq2Toq0(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return qx2qy(_grid, _q2, 2, 0);
}

/**
 * \brief Performs conversion from q2 to q3 distributions.
 * \details Using information about the size grid by \f$q_{3,i} = \frac{d_i q_{2,i}}{M_1(q_2)}\f$.
 * Refer to function qx2qy(const std::vector<double>&, const std::vector<double>&, int, int).
 * \param _grid Distribution grid.
 * \param _q2 Input distribution.
 * \return Converted q3 distribution.
 */
std::vector<double> inline Convertq2Toq3(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return qx2qy(_grid, _q2, 2, 3);
}

/**
 * \brief Performs conversion from q3 to q0 distributions.
 * \details Using information about the size grid by \f$q_{0,i} = \frac{d_i^{-3} q_{3,i}}{M_{-3}(q_3)}\f$.
 * Refer to function qx2qy(const std::vector<double>&, const std::vector<double>&, int, int).
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Converted q0 distribution.
 */
std::vector<double> inline Convertq3Toq0(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return qx2qy(_grid, _q3, 3, 0);
}

/**
 * \brief Performs conversion from q3 to q2 distributions.
 * \details Using information about the size grid by \f$q_{2,i} = \frac{d_i^{-1} q_{3,i}}{M_{-1}(q_3)}\f$.
 * Refer to function qx2qy(const std::vector<double>&, const std::vector<double>&, int, int).
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Converted q2 distribution.
 */
std::vector<double> inline Convertq3Toq2(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return qx2qy(_grid, _q3, 3, 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Calculates q3 distribution.
 * \details Using the size grid and the distribution of mass fractions by \f$q_3 = w_i / \Delta d_i\f$.
 * \param _grid Distribution grid.
 * \param _massFrac Input distribution.
 * \return Calculated q3 distribution.
 */
std::vector<double> inline ConvertMassFractionsToq3(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	std::vector<double> q3(_massFrac.size());
	for (size_t i = 0; i < _massFrac.size(); ++i)
		q3[i] = _massFrac[i] / (_grid[i + 1] - _grid[i]);
	return q3;
}

/**
 * \brief Calculates Q3 distribution.
 * \details Using the distribution of mass fractions: \f$Q_{3,0} = w_i\f$ and \f$Q_{3,i} = Q_{3,i-1} + w_i\f$.
 * \param _massFrac Input distribution.
 * \return Calculated Q3 distribution.
 */
std::vector<double> inline ConvertMassFractionsToQ3(const std::vector<double>& _massFrac)
{
	if (_massFrac.empty()) return {};
	std::vector<double> Q3(_massFrac.size());
	Q3[0] = _massFrac[0];
	for (size_t i = 1; i < _massFrac.size(); ++i)
		Q3[i] = Q3[i - 1] + _massFrac[i];
	return Q3;
}

/**
 * \brief Calculates q2 distribution.
 * \details Refer to the functions ConvertMassFractionsToq3(const std::vector<double>&, const std::vector<double>&) and Convertq3Toq2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _massFrac Input distribution.
 * \return Calculated q2 distribution.
 */
std::vector<double> inline ConvertMassFractionsToq2(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq3Toq2(_grid, ConvertMassFractionsToq3(_grid, _massFrac));
}

/**
 * \brief Calculates Q2 distribution.
 * \details Refer to the functions ConvertMassFractionsToq2(const std::vector<double>&, const std::vector<double>&) and Convertq2ToQ2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _massFrac Input distribution.
 * \return Calculated Q2 distribution.
 */
std::vector<double> inline ConvertMassFractionsToQ2(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq2ToQ2(_grid, ConvertMassFractionsToq2(_grid, _massFrac));
}

/**
 * \brief Calculates q0 distribution.
 * \details Refer to the functions ConvertMassFractionsToq3(const std::vector<double>&, const std::vector<double>&) and Convertq3Toq0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _massFrac Input distribution.
 * \return Calculated q0 distribution.
 */
std::vector<double> inline ConvertMassFractionsToq0(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq3Toq0(_grid, ConvertMassFractionsToq3(_grid, _massFrac));
}

/**
 * \brief Calculates Q0 distribution.
 * \details Refer to the functions ConvertMassFractionsToq0(const std::vector<double>&, const std::vector<double>&) and Convertq0ToQ0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _massFrac Input distribution.
 * \return Calculated q0 distribution.
 */
std::vector<double> inline ConvertMassFractionsToQ0(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq0ToQ0(_grid, ConvertMassFractionsToq0(_grid, _massFrac));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Calculates mass fractions from q3.
 * \details Using the size grid by \f$w_i = q_{3,i}\cdot \Delta d_i\f$.
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Calculated mass fractions.
 */
std::vector<double> inline Convertq3ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	std::vector<double> massFrac(_q3.size());
	for (size_t i = 0; i < _q3.size(); ++i)
		massFrac[i] = _q3[i] * (_grid[i + 1] - _grid[i]);
	return massFrac;
}

/**
 * \brief Calculates mass fractions from Q3 distribution.
 * \details Using the size grid: \f$w_0 = Q_{3,0}\f$ and \f$w_i = Q_{3,i} - Q_{3,i-1}\f$.
 * \param _Q3 Input distribution.
 * \return Calculated mass fractions.
 */
std::vector<double> inline ConvertQ3ToMassFractions(const std::vector<double>& _Q3)
{
	if (_Q3.empty()) return {};
	std::vector<double> massFrac(_Q3.size());
	massFrac[0] = _Q3[0];
	for (size_t i = 1; i < _Q3.size(); ++i)
		massFrac[i] = _Q3[i] - _Q3[i - 1];
	return massFrac;
}

/**
 * \brief Calculates mass fractions from q2 distribution.
 * \details Refer to the functions Convertq2Toq3(const std::vector<double>&, const std::vector<double>&) and Convertq3ToMassFractions(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q2 Input distribution.
 * \return Calculated mass fractions.
 */
std::vector<double> inline Convertq2ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return Convertq3ToMassFractions(_grid, Convertq2Toq3(_grid, _q2));
}

/**
 * \brief Calculates mass fractions from Q2 distribution.
 * \details Refer to the functions ConvertQ2Toq2(const std::vector<double>&, const std::vector<double>&) and Convertq2ToMassFractions(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q2 Input distribution.
 * \return Calculated mass fractions.
 */
std::vector<double> inline ConvertQ2ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return Convertq2ToMassFractions(_grid, ConvertQ2Toq2(_grid, _Q2));
}

/**
 * \brief Calculates mass fractions from q0 distribution.
 * \details Refer to the functions Convertq0Toq3(const std::vector<double>&, const std::vector<double>&) and Convertq3ToMassFractions(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q0 Input distribution.
 * \return Calculated mass fractions.
 */
std::vector<double> inline Convertq0ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return Convertq3ToMassFractions(_grid, Convertq0Toq3(_grid, _q0));
}

/**
 * \brief Calculates mass fractions from Q0 distribution.
 * \details Refer to the functions ConvertQ0Toq0(const std::vector<double>&, const std::vector<double>&) and Convertq0ToMassFractions(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Calculated mass fractions.
 */
std::vector<double> inline ConvertQ0ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return Convertq0ToMassFractions(_grid, ConvertQ0Toq0(_grid, _Q0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Calculates q0 distribution using the number distribution and the size grid.
 * \details \f$q_{0,i} = \frac{N_i}{\Delta d_i N_{tot}}\f$.
 * \param _grid Distribution grid.
 * \param _number Number distribution.
 * \return Calculated q0 distribution.
 */
std::vector<double> inline ConvertNumbersToq0(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	const double Ntot = std::accumulate(_number.begin(), _number.end(), 0.0);
	if (Ntot == 0.0) return std::vector(_number.size(), 0.0);
	std::vector<double> q0(_number.size());
	for (size_t i = 0; i < _number.size(); ++i)
		q0[i] = _number[i] / Ntot / (_grid[i + 1] - _grid[i]);
	return q0;
}

/**
 * \brief Calculates Q0 distribution using the number distribution.
 * \details Refer to the functions ConvertNumbersToq0(const std::vector<double>&, const std::vector<double>&) and Convertq0ToQ0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _number Number distribution.
 * \return Calculated Q0 distribution.
 */
std::vector<double> inline ConvertNumbersToQ0(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq0ToQ0(_grid, ConvertNumbersToq0(_grid, _number));
}

/**
 * \brief Calculates Q2 distribution using the number distribution and the size grid.
 * \details \f$Q_{2,i} = \frac{\sum_{j=0}^i N_j \pi d_j^2}{\sum_j N_j \pi d_j^2}\f$.
 * \param _grid Distribution grid.
 * \param _number Number distribution.
 * \return Calculated Q2 distribution.
 */
std::vector<double> inline ConvertNumbersToQ2(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	if (_number.empty()) return {};
	if (_grid.size() != _number.size() + 1) return {};
	constexpr double PI = 3.14159265358979323846;
	std::vector<double> Q2(_number.size());
	Q2[0] = _number[0] * PI * pow((_grid[0] + _grid[1]) / 2, 2);
	for (size_t i = 1; i < _number.size(); ++i)
		Q2[i] = Q2[i - 1] + _number[i] * PI * pow((_grid[i] + _grid[i + 1]) / 2, 2);
	for (size_t i = 0; i < Q2.size(); ++i)
		Q2[i] /= Q2.back();
	return Q2;
}

/**
 * \brief Calculates q2 distribution using the number distribution.
 * \details Refer to the functions ConvertNumbersToQ2(const std::vector<double>&, const std::vector<double>&) and ConvertQ2Toq2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _number Number distribution.
 * \return Calculated q2 distribution.
 */
std::vector<double> inline ConvertNumbersToq2(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return ConvertQ2Toq2(_grid, ConvertNumbersToQ2(_grid, _number));
}

/**
 * \brief Calculates q3 distribution using the number distribution.
 * \details Refer to the functions ConvertNumbersToq0(const std::vector<double>&, const std::vector<double>&) and Convertq0Toq3(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _number Number distribution.
 * \return Calculated q3 distribution.
 */
std::vector<double> inline ConvertNumbersToq3(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq0Toq3(_grid, ConvertNumbersToq0(_grid, _number));
}

/**
 * \brief Calculates Q3 distribution using the number distribution.
 * \details Refer to the functions ConvertNumbersToq3(const std::vector<double>&, const std::vector<double>&) and Convertq3ToQ3(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _number Number distribution.
 * \return Calculated Q3 distribution.
 */
std::vector<double> inline ConvertNumbersToQ3(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq3ToQ3(_grid, ConvertNumbersToq3(_grid, _number));
}

/**
 * \brief Calculates mass fractions from the number distribution.
 * \details Refer to the functions ConvertNumbersToq0(const std::vector<double>&, const std::vector<double>&) and Convertq0ToMassFractions(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _number Number distribution.
 * \return Calculated mass fractions.
 */
std::vector<double> inline ConvertNumbersToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq0ToMassFractions(_grid, ConvertNumbersToq0(_grid, _number));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Calculates Q2 distribution.
 * \details Refer to the functions Convertq0Toq2(const std::vector<double>&, const std::vector<double>&) and Convertq2ToQ2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q0 Input distribution.
 * \return Calculated Q2 distribution.
 */
std::vector<double> inline Convertq0ToQ2(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return Convertq2ToQ2(_grid, Convertq0Toq2(_grid, _q0));
}

/**
 * \brief Calculates Q3 distribution.
 * \details Refer to the functions Convertq0Toq3(const std::vector<double>&, const std::vector<double>&) and Convertq3ToQ3(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q0 Input distribution.
 * \return Calculated Q3 distribution.
 */
std::vector<double> inline Convertq0ToQ3(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return Convertq3ToQ3(_grid, Convertq0Toq3(_grid, _q0));
}

/**
 * \brief Calculates Q0 distribution.
 * \details Refer to the functions Convertq2Toq0(const std::vector<double>&, const std::vector<double>&) and Convertq0ToQ0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q2 Input distribution.
 * \return Calculated Q0 distribution.
 */
std::vector<double> inline Convertq2ToQ0(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return Convertq0ToQ0(_grid, Convertq2Toq0(_grid, _q2));
}

/**
 * \brief Calculates Q3 distribution.
 * \details Refer to the functions Convertq2Toq3(const std::vector<double>&, const std::vector<double>&) and Convertq3ToQ3(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q2 Input distribution.
 * \return Calculated Q3 distribution.
 */
std::vector<double> inline Convertq2ToQ3(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return Convertq3ToQ3(_grid, Convertq2Toq3(_grid, _q2));
}

/**
 * \brief Calculates Q0 distribution.
 * \details Refer to the functions Convertq3Toq0(const std::vector<double>&, const std::vector<double>&) and Convertq0ToQ0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Calculated Q0 distribution.
 */
std::vector<double> inline Convertq3ToQ0(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return Convertq0ToQ0(_grid, Convertq3Toq0(_grid, _q3));
}

/**
 * \brief Calculates Q2 distribution.
 * \details Refer to the functions Convertq3Toq2(const std::vector<double>&, const std::vector<double>&) and Convertq2ToQ2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Calculated Q2 distribution.
 */
std::vector<double> inline Convertq3ToQ2(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return Convertq2ToQ2(_grid, Convertq3Toq2(_grid, _q3));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Calculates q2 distribution.
 * \details Refer to the functions ConvertQ0Toq0(const std::vector<double>&, const std::vector<double>&) and Convertq0Toq2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Calculated q2 distribution.
 */
std::vector<double> inline ConvertQ0Toq2(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return Convertq0Toq2(_grid, ConvertQ0Toq0(_grid, _Q0));
}

/**
 * \brief Calculates q3 distribution.
 * \details Refer to the functions ConvertQ0Toq0(const std::vector<double>&, const std::vector<double>&) and Convertq0Toq3(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Calculated q3 distribution.
 */
std::vector<double> inline ConvertQ0Toq3(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return Convertq0Toq3(_grid, ConvertQ0Toq0(_grid, _Q0));
}

/**
 * \brief Calculates q0 distribution.
 * \details Refer to the functions ConvertQ2Toq2(const std::vector<double>&, const std::vector<double>&) and Convertq2Toq0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q2 Input distribution.
 * \return Calculated q0 distribution.
 */
std::vector<double> inline ConvertQ2Toq0(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return Convertq2Toq0(_grid, ConvertQ2Toq2(_grid, _Q2));
}

/**
 * \brief Calculates q3 distribution.
 * \details Refer to the functions ConvertQ2Toq2(const std::vector<double>&, const std::vector<double>&) and Convertq2Toq3(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q2 Input distribution.
 * \return Calculated q3 distribution.
 */
std::vector<double> inline ConvertQ2Toq3(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return Convertq2Toq3(_grid, ConvertQ2Toq2(_grid, _Q2));
}

/**
 * \brief Calculates q0 distribution.
 * \details Refer to the functions ConvertQ3Toq3(const std::vector<double>&, const std::vector<double>&) and Convertq3Toq0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q3 Input distribution.
 * \return Calculated q0 distribution.
 */
std::vector<double> inline ConvertQ3Toq0(const std::vector<double>& _grid, const std::vector<double>& _Q3)
{
	return Convertq3Toq0(_grid, ConvertQ3Toq3(_grid, _Q3));
}

/**
 * \brief Calculates q2 distribution.
 * \details Refer to the functions ConvertQ3Toq3(const std::vector<double>&, const std::vector<double>&) and Convertq3Toq2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q3 Input distribution.
 * \return Calculated q2 distribution.
 */
std::vector<double> inline ConvertQ3Toq2(const std::vector<double>& _grid, const std::vector<double>& _Q3)
{
	return Convertq3Toq2(_grid, ConvertQ3Toq3(_grid, _Q3));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Calculates Q2 distribution.
 * \details Refer to the functions ConvertQ0ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToQ2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Calculated Q2 distribution.
 */
std::vector<double> inline ConvertQ0ToQ2(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return ConvertMassFractionsToQ2(_grid, ConvertQ0ToMassFractions(_grid, _Q0));
}

/**
 * \brief Calculates Q3 distribution.
 * \details Refer to the functions ConvertQ0ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToQ3(const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Calculated Q3 distribution.
 */
std::vector<double> inline ConvertQ0ToQ3(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return ConvertMassFractionsToQ3(ConvertQ0ToMassFractions(_grid, _Q0));
}

/**
 * \brief Calculates Q0 distribution.
 * \details Refer to the functions ConvertQ2ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToQ0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q2 Input distribution.
 * \return Calculated Q0 distribution.
 */
std::vector<double> inline ConvertQ2ToQ0(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return ConvertMassFractionsToQ0(_grid, ConvertQ2ToMassFractions(_grid, _Q2));
}

/**
 * \brief Calculates Q3 distribution.
 * \details Refer to the functions ConvertQ2ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToQ3(const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q2 Input distribution.
 * \return Calculated Q3 distribution.
 */
std::vector<double> inline ConvertQ2ToQ3(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return ConvertMassFractionsToQ3(ConvertQ2ToMassFractions(_grid, _Q2));
}

/**
 * \brief Calculates Q0 distribution.
 * \details Refer to the functions ConvertQ0ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToQ0(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Calculated Q0 distribution.
 */
std::vector<double> inline ConvertQ3ToQ0(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return ConvertMassFractionsToQ0(_grid, ConvertQ0ToMassFractions(_grid, _Q0));
}

/**
 * \brief Calculates Q2 distribution.
 * \details Refer to the functions ConvertQ3ToMassFractions(const std::vector<double>&) and ConvertMassFractionsToQ2(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q3 Input distribution.
 * \return Calculated Q2 distribution.
 */
std::vector<double> inline ConvertQ3ToQ2(const std::vector<double>& _grid, const std::vector<double>& _Q3)
{
	return ConvertMassFractionsToQ2(_grid, ConvertQ3ToMassFractions(_Q3));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Assumes unity density and unity total mass.
 * \details \f$N_i = \frac{6 \cdot w_i}{\pi \cdot \Delta d_i^3}\f$.
 * \param _grid Distribution grid.
 * \param _massFrac Input distribution.
 * \return Number distribution.
 */
std::vector<double> inline ConvertMassFractionsToNumbers(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	constexpr double PI = 3.14159265358979323846;
	std::vector res(_massFrac.size(), 0.0);
	for (size_t i = 0; i < _massFrac.size(); ++i)
		res[i] = 6 * _massFrac[i] / (PI * pow(_grid[i + 1] - _grid[i], 3));
	return res;
}

/**
 * \brief Assumes unity density and unity total mass.
 * \details Refer to the functions Convertq0ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToNumbers(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q0 Input distribution.
 * \return Number distribution.
 */
std::vector<double> inline Convertq0ToNumbers(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return ConvertMassFractionsToNumbers(_grid, Convertq0ToMassFractions(_grid, _q0));
}

/**
 * \brief Assumes unity density and unity total mass.
 * \details Refer to the functions Convertq2ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToNumbers(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q2 Input distribution.
 * \return Number distribution.
 */
std::vector<double> inline Convertq2ToNumbers(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return ConvertMassFractionsToNumbers(_grid, Convertq2ToMassFractions(_grid, _q2));
}

/**
 * \brief Assumes unity density and unity total mass.
 * \details Refer to the functions Convertq3ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToNumbers(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Number distribution.
 */
std::vector<double> inline Convertq3ToNumbers(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return ConvertMassFractionsToNumbers(_grid, Convertq3ToMassFractions(_grid, _q3));
}

/**
 * \brief Assumes unity density and unity total mass.
 * \details Refer to the functions ConvertQ0ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToNumbers(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q0 Input distribution.
 * \return Number distribution.
 */
std::vector<double> inline ConvertQ0ToNumbers(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return ConvertMassFractionsToNumbers(_grid, ConvertQ0ToMassFractions(_grid, _Q0));
}

/**
 * \brief Assumes unity density and unity total mass.
 * \details Refer to the functions ConvertQ2ToMassFractions(const std::vector<double>&, const std::vector<double>&) and ConvertMassFractionsToNumbers(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q2 Input distribution.
 * \return Number distribution.
 */
std::vector<double> inline ConvertQ2ToNumbers(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return ConvertMassFractionsToNumbers(_grid, ConvertQ2ToMassFractions(_grid, _Q2));
}

/**
 * \brief Assumes unity density and unity total mass.
 * \details Refer to the functions ConvertQ3ToMassFractions(const std::vector<double>&) and ConvertMassFractionsToNumbers(const std::vector<double>&, const std::vector<double>&).
 * \param _grid Distribution grid.
 * \param _Q3 Input distribution.
 * \return Number distribution.
 */
std::vector<double> inline ConvertQ3ToNumbers(const std::vector<double>& _grid, const std::vector<double>& _Q3)
{
	return ConvertMassFractionsToNumbers(_grid, ConvertQ3ToMassFractions(_Q3));
}


////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Converts the mass fraction distribution defined on the numeric grid to the new grid.
 * \param _grid Old mass fraction distribution.
 * \param _w Old symbolic distribution grid.
 * \param _gridNew New distribution grid.
 * \return Converted distribution on the modified size grid.
 */
std::vector<double> inline ConvertOnNewGrid(const std::vector<std::string>& _grid, const std::vector<double>& _w, const std::vector<std::string>& _gridNew)
{
	if (_grid == _gridNew) return _w;
	std::vector<double> res(_gridNew.size());
	for (size_t i = 0; i < _gridNew.size(); ++i)
	{
		const auto it = std::find(_grid.begin(), _grid.end(), _gridNew[i]);
		res[i] = it != _grid.end() ? _w[std::distance(_grid.begin(), it)] : 0.0;
	}
	return res;
}

/**
 * \brief Converts the mass fraction distribution defined on the numeric grid to the new grid.
 * \param _grid Old mass fraction distribution.
 * \param _w Old numeric distribution grid.
 * \param _gridNew New distribution grid.
 * \return Converted distribution on the modified size grid.
 */
std::vector<double> inline ConvertOnNewGrid(const std::vector<double>& _grid, const std::vector<double>& _w, const std::vector<double>& _gridNew)
{
	if (_grid == _gridNew) return _w;
	if (_gridNew.empty()) return {};
	if (std::all_of(_w.begin(), _w.end(), [](double d) { return d == 0.0; })) return std::vector<double>(_gridNew.size() - 1, 0.0);
	const std::vector<double> QDistr = ConvertMassFractionsToQ3(_w);
	std::vector<double> res(_gridNew.size() - 1);
	double Q1 = GetQ(QDistr, _grid, _gridNew[0]);
	for (size_t i = 0; i < _gridNew.size() - 1; ++i)
	{
		const double Q2 = GetQ(QDistr, _grid, _gridNew[i + 1]);
		res[i] = (Q2 - Q1) / (_gridNew[i + 1] - _gridNew[i]);
		Q1 = Q2;
	}
	return res;
}

/**
 * \brief Converts density distribution defined on the numeric grid to the new grid.
 * \param _grid Old numeric distribution grid.
 * \param _q Old density distribution.
 * \param _gridNew New distribution grid.
 * \return Converted distribution on the modified size grid.
 */
std::vector<double> inline ConvertqOnNewGrid(const std::vector<double>& _grid, const std::vector<double>& _q, const std::vector<double>& _gridNew)
{
	if (_grid == _gridNew) return _q;
	std::vector<double> qNew(_gridNew.size() - 1);
	const std::vector<double> QDistr = q2Q(_grid, _q);
	double Q1 = GetQ(QDistr, _grid, _gridNew[0]);
	for (size_t i = 0; i < _gridNew.size() - 1; ++i)
	{
		const double Q2 = GetQ(QDistr, _grid, _gridNew[i + 1]);
		qNew[i] = (Q2 - Q1) / (_gridNew[i + 1] - _gridNew[i]);
		Q1 = Q2;
	}
	return qNew;
}

/**
 * \brief Converts q0 distribution to the same distribution on the modified size grid.
 * \details Refer to function ConvertqOnNewGrid(const std::vector<double>&, const std::vector<double>&, const std::vector<double>&).
 * \param _gridOld Old distribution grid.
 * \param _q0Old Old distribution.
 * \param _gridNew New distribution grid.
 * \return Converted q0 distribution on the modified size grid.
 */
std::vector<double> inline Convertq0Toq0(const std::vector<double>& _gridOld, const std::vector<double>& _q0Old, std::vector<double>& _gridNew)
{
	return ConvertqOnNewGrid(_gridOld, _q0Old, _gridNew);
}

/**
 * \brief Converts q2 distribution to the same distribution on the modified size grid.
 * \details Refer to function ConvertqOnNewGrid(const std::vector<double>&, const std::vector<double>&, const std::vector<double>&).
 * \param _gridOld Old distribution grid.
 * \param _q2Old Old distribution.
 * \param _gridNew New distribution grid.
 * \return Converted q2 distribution on the modified size grid.
 */
std::vector<double> inline Convertq2Toq2(const std::vector<double>& _gridOld, const std::vector<double>& _q2Old, std::vector<double>& _gridNew)
{
	return ConvertqOnNewGrid(_gridOld, _q2Old, _gridNew);
}

/**
 * \brief Converts q3 distribution to the same distribution on the modified size grid.
 * \details Refer to function ConvertqOnNewGrid(const std::vector<double>&, const std::vector<double>&, const std::vector<double>&).
 * \param _gridOld Old distribution grid.
 * \param _q3Old Old distribution.
 * \param _gridNew New distribution grid.
 * \return Converted q3 distribution on the modified size grid.
 */
std::vector<double> inline Convertq3Toq3(const std::vector<double>& _gridOld, const std::vector<double>& _q3Old, std::vector<double>& _gridNew)
{
	return ConvertqOnNewGrid(_gridOld, _q3Old, _gridNew);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Normalizes density distribution q0 or q3.
 * \details \f$q_i = \frac{q_i}{\sum_j q_j \Delta d_j}\f$.
 * \param _grid Distribution grid.
 * \param _qiDistr Input distribution.
 */
void inline NormalizeDensityDistribution(const std::vector<double>& _grid, std::vector<double>& _qiDistr)
{
	// normalization of the distribution
	double dSum = 0;
	for (size_t i = 0; i<_qiDistr.size(); i++)
		dSum += _qiDistr[i] * (_grid[i + 1] - _grid[i]);
	if (dSum == 0)
		for (double& v : _qiDistr)
			v = 1. / _grid.size();
	else
		for (double& v : _qiDistr)
			v = v / dSum;
}

/**
 * \brief Returns diameter in [m], which corresponds to a specified value of cumulative distribution Q0 or Q3.
 * \details Input value should range between 0 and 1.
 * \param _grid Distribution grid.
 * \param _QiDistr Input distribution.
 * \param _val Value in range between 0 and 1.
 * \return Distribution value.
 */
double inline GetDistributionValue(const std::vector<double>& _grid, const std::vector<double>& _QiDistr, double _val)
{
	if (_val < 0 || _val > 1) return 0;
	if (_QiDistr.empty()) return 0; // if no element at all
	if (_QiDistr.size() == 1) return (_grid[1] + _grid[0]) * _val; // if just one element

	/// find two elements to interpolate values
	size_t nLeft = 0;
	while (nLeft < _QiDistr.size() - 1 && (_QiDistr[nLeft] < _val))
		nLeft++;
	if (nLeft == 0)
		return 0;
	nLeft--;
	const size_t nRight = nLeft + 1;

	const double dDLeft = (_grid[nLeft] + _grid[nLeft + 1]) / 2; // diameter of left interval
	const double dDRight = (_grid[nRight] + _grid[nRight + 1]) / 2; // diameter of right interval
	const double dQLeft = _QiDistr[nLeft];
	const double dQRight = _QiDistr[nRight];

	if (dQLeft == dQRight)
		return (dDLeft + dDRight) * _val;
	else
		return dDLeft + (dDRight - dDLeft)*(_val - dQLeft) / (dQRight - dQLeft);
}

/**
 * \brief Returns median in [m] of Q0 or Q3 distribution. Median is a diameter, which corresponds to a value of distribution equal to 0.5.
 * \details Refer to function GetDistributionValue(const std::vector<double>&, const std::vector<double>&, double).
 * \param _grid Distribution grid.
 * \param _QiDistr Input distribution.
 * \return Median of distribution.
 */
double inline GetDistributionMedian(const std::vector<double>& _grid, const std::vector<double>& _QiDistr)
{
	return GetDistributionValue(_grid, _QiDistr, 0.5);
}

/**
 * \brief Returns diameter in [m], which corresponds to a maximum value of density distribution.
 * \details
 * \param _grid Distribution grid.
 * \param _qiDistr Input distribution.
 * \return Mode of distribution.
 */
double inline GetDistributionMode(const std::vector<double>& _grid, const std::vector<double>& _qiDistr)
{
	if (_grid.size() != _qiDistr.size() + 1) return {};
	double dMaxDiameter = 0;
	double dMaxqValue = 0;
	for (size_t i = 0; i < _qiDistr.size(); ++i)
		if (_qiDistr[i] > dMaxqValue)
		{
			dMaxqValue = _qiDistr[i];
			dMaxDiameter = (_grid[i] + _grid[i + 1]) / 2;
		}
	return dMaxDiameter;
}

/**
 * \brief Returns average diameter in [m] of the distribution q0 or q3.
 * \details
 * \param _grid Distribution grid.
 * \param _qiDistr Input distribution.
 * \return Average diameter of distribution.
 */
double inline GetAverageDiameter(const std::vector<double>& _grid, const std::vector<double>& _qiDistr)
{
	double dResult = 0;
	for (size_t i = 0; i < _qiDistr.size(); ++i)
		dResult += _qiDistr[i] * ((_grid[i] + _grid[i + 1]) / 2) * (_grid[i + 1] - _grid[i]);
	return dResult;
}

/**
 * \brief Calculates specific surface of q3 distribution in [m<sup>2</sup>].
 * \details
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Specific surface of distribution.
 */
double inline GetSpecificSurface(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	double dSV = 0;
	for (size_t i = 0; i < _q3.size(); i++)
		dSV += 6 * _q3[i] * 1 / ((_grid[i] + _grid[i + 1]) / 2) * (_grid[i + 1] - _grid[i]);
	return dSV;
}

/**
 * \brief Calculates Sauter diameter (d<sub>32</sub>) of q3 distribution in [m].
 * \details
 * \param _grid Distribution grid.
 * \param _q3 Input distribution.
 * \return Sauter diameter of distribution.
 */
double inline GetSauterDiameter(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	if (_q3.size() + 1 != _grid.size())
		return 0;
	double dSum = 0;
	for (size_t i = 0; i < _q3.size(); ++i)
		dSum += _q3[i] * (_grid[i + 1] - _grid[i]) / ((_grid[i] + _grid[i + 1]) / 2);
	if (dSum != 0)
		dSum = 1 / dSum;
	return dSum;
}