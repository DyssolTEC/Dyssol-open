/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>

double inline GetMMoment(int _moment, const std::vector<double>& _grid, const std::vector<double>& _q)
{
	if (_grid.size() != _q.size() + 1)	return 0;

	double dMoment = 0;
	for (size_t i = 0; i < _q.size(); ++i)
		dMoment += std::pow((_grid[i] + _grid[i + 1]) / 2, _moment) * _q[i] * (_grid[i + 1] - _grid[i]);
	return dMoment;
}

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

std::vector<double> inline Q2q(const std::vector<double>& _grid, const std::vector<double>& _Q)
{
	if (_Q.empty()) return {};
	std::vector<double> q(_Q.size());
	q[0] = _Q[0] / (_grid[1] - _grid[0]);
	for (size_t i = 1; i < _Q.size(); ++i)
		q[i] = (_Q[i] - _Q[i - 1]) / (_grid[i + 1] - _grid[i]);
	return q;
}

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

std::vector<double> inline qx2qy(const std::vector<double>& _grid, const std::vector<double>& _qx, int x, int y)
{
	if (_grid.size() != _qx.size() + 1) return {};

	const double M = GetMMoment(y - x, _grid, _qx);
	if (M == 0) return _qx;
	std::vector<double> qy(_qx.size());
	for (size_t i = 0; i < _qx.size(); ++i)
		qy[i] = std::pow((_grid[i] + _grid[i + 1]) / 2, y - x) * _qx[i] / M;
	return qy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<double> inline ConvertQ0Toq0(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return Q2q(_grid, _Q0);
}

std::vector<double> inline Convertq0ToQ0(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return q2Q(_grid, _q0);
}

std::vector<double> inline ConvertQ2Toq2(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return Q2q(_grid, _Q2);
}

std::vector<double> inline Convertq2ToQ2(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return q2Q(_grid, _q2);
}

std::vector<double> inline ConvertQ3Toq3(const std::vector<double>& _grid, const std::vector<double>& _Q3)
{
	return Q2q(_grid, _Q3);
}

std::vector<double> inline Convertq3ToQ3(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return q2Q(_grid, _q3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<double> inline Convertq0Toq2(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return qx2qy(_grid, _q0, 0, 2);
}

std::vector<double> inline Convertq0Toq3(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return qx2qy(_grid, _q0, 0, 3);
}

std::vector<double> inline Convertq2Toq0(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return qx2qy(_grid, _q2, 2, 0);
}

std::vector<double> inline Convertq2Toq3(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return qx2qy(_grid, _q2, 2, 3);
}

std::vector<double> inline Convertq3Toq0(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return qx2qy(_grid, _q3, 3, 0);
}

std::vector<double> inline Convertq3Toq2(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	return qx2qy(_grid, _q3, 3, 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<double> inline ConvertMassFractionsToq3(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	std::vector<double> q3(_massFrac.size());
	for (size_t i = 0; i < _massFrac.size(); ++i)
		q3[i] = _massFrac[i] / (_grid[i + 1] - _grid[i]);
	return q3;
}

std::vector<double> inline ConvertMassFractionsToQ3(const std::vector<double>& _massFrac)
{
	if (_massFrac.empty()) return {};
	std::vector<double> Q3(_massFrac.size());
	Q3[0] = _massFrac[0];
	for (size_t i = 1; i < _massFrac.size(); ++i)
		Q3[i] = Q3[i - 1] + _massFrac[i];
	return Q3;
}

std::vector<double> inline ConvertMassFractionsToq2(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq3Toq2(_grid, ConvertMassFractionsToq3(_grid, _massFrac));
}

std::vector<double> inline ConvertMassFractionsToQ2(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq2ToQ2(_grid, ConvertMassFractionsToq2(_grid, _massFrac));
}

std::vector<double> inline ConvertMassFractionsToq0(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq3Toq0(_grid, ConvertMassFractionsToq3(_grid, _massFrac));
}

std::vector<double> inline ConvertMassFractionsToQ0(const std::vector<double>& _grid, const std::vector<double>& _massFrac)
{
	return Convertq0ToQ0(_grid, ConvertMassFractionsToq0(_grid, _massFrac));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<double> inline Convertq3ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	std::vector<double> massFrac(_q3.size());
	for (size_t i = 0; i < _q3.size(); ++i)
		massFrac[i] = _q3[i] * (_grid[i + 1] - _grid[i]);
	return massFrac;
}

std::vector<double> inline ConvertQ3ToMassFractions(const std::vector<double>& _Q3)
{
	if (_Q3.empty()) return {};
	std::vector<double> massFrac(_Q3.size());
	massFrac[0] = _Q3[0];
	for (size_t i = 1; i < _Q3.size(); ++i)
		massFrac[i] = _Q3[i] - _Q3[i - 1];
	return massFrac;
}

std::vector<double> inline Convertq2ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _q2)
{
	return Convertq3ToMassFractions(_grid, Convertq2Toq3(_grid, _q2));
}

std::vector<double> inline ConvertQ2ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _Q2)
{
	return Convertq2ToMassFractions(_grid, ConvertQ2Toq2(_grid, _Q2));
}

std::vector<double> inline Convertq0ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _q0)
{
	return Convertq3ToMassFractions(_grid, Convertq0Toq3(_grid, _q0));
}

std::vector<double> inline ConvertQ0ToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _Q0)
{
	return Convertq0ToMassFractions(_grid, ConvertQ0Toq0(_grid, _Q0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<double> inline ConvertNumbersToq0(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	const double Ntot = std::accumulate(_number.begin(), _number.end(), 0.0);
	if (Ntot == 0.0) return std::vector(_number.size(), 0.0);
	std::vector<double> q0(_number.size());
	for (size_t i = 0; i < _number.size(); ++i)
		q0[i] = _number[i] / Ntot / (_grid[i + 1] - _grid[i]);
	return q0;
}

std::vector<double> inline ConvertNumbersToQ0(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq0ToQ0(_grid, ConvertNumbersToq0(_grid, _number));
}

std::vector<double> inline ConvertNumbersToQ2(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	if (_number.empty()) return {};
	if (_grid.size() != _number.size() + 1) return {};
	const double PI = 3.14159265358979323846;
	std::vector<double> Q2(_number.size());
	Q2[0] = _number[0] * PI * pow((_grid[0] + _grid[1]) / 2, 2);
	for (size_t i = 1; i < _number.size(); ++i)
		Q2[i] = Q2[i - 1] + _number[i] * PI * pow((_grid[i] + _grid[i + 1]) / 2, 2);
	for (size_t i = 0; i < Q2.size(); ++i)
		Q2[i] /= Q2.back();
	return Q2;
}

std::vector<double> inline ConvertNumbersToq2(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return ConvertQ2Toq2(_grid, ConvertNumbersToQ2(_grid, _number));
}

std::vector<double> inline ConvertNumbersToq3(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq0Toq3(_grid, ConvertNumbersToq0(_grid, _number));
}

std::vector<double> inline ConvertNumbersToQ3(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq3ToQ3(_grid, ConvertNumbersToq3(_grid, _number));
}

std::vector<double> inline ConvertNumbersToMassFractions(const std::vector<double>& _grid, const std::vector<double>& _number)
{
	return Convertq0ToMassFractions(_grid, ConvertNumbersToq0(_grid, _number));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Converts the mass fraction distribution _w defined on the symbolic _grid to the new grid _gridNew.
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

// Converts the mass fraction distribution _w defined on the numeric _grid to the new grid _gridNew.
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

// Converts density distribution _q defined on the numeric _grid to the new grid _gridNew.
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

std::vector<double> inline Convertq0Toq0(const std::vector<double>& _gridOld, const std::vector<double>& _q0Old, std::vector<double>& _gridNew)
{
	return ConvertqOnNewGrid(_gridOld, _q0Old, _gridNew);
}

std::vector<double> inline Convertq2Toq2(const std::vector<double>& _gridOld, const std::vector<double>& _q2Old, std::vector<double>& _gridNew)
{
	return ConvertqOnNewGrid(_gridOld, _q2Old, _gridNew);
}

std::vector<double> inline Convertq3Toq3(const std::vector<double>& _gridOld, const std::vector<double>& _q3Old, std::vector<double>& _gridNew)
{
	return ConvertqOnNewGrid(_gridOld, _q3Old, _gridNew);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

double inline GetDistributionValue(const std::vector<double>& _grid, const std::vector<double>& _QiDistr, double val)
{
	if (val < 0 || val > 1) return 0;
	if (_QiDistr.empty()) return 0; // if no element at all
	if (_QiDistr.size() == 1) return (_grid[1] + _grid[0]) * val; // if just one element

	/// find two elements to interpolate values
	size_t nLeft = 0;
	while (nLeft < _QiDistr.size() - 1 && (_QiDistr[nLeft] < val))
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
		return (dDLeft + dDRight) * val;
	else
		return dDLeft + (dDRight - dDLeft)*(val - dQLeft) / (dQRight - dQLeft);
}

double inline GetDistributionMedian(const std::vector<double>& _grid, const std::vector<double>& _QiDistr)
{
	return GetDistributionValue(_grid, _QiDistr, 0.5);
}

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

double inline GetAverageDiameter(const std::vector<double>& _grid, const std::vector<double>& _qiDistr)
{
	double dResult = 0;
	for (size_t i = 0; i < _qiDistr.size(); ++i)
		dResult += _qiDistr[i] * ((_grid[i] + _grid[i + 1]) / 2) * (_grid[i + 1] - _grid[i]);
	return dResult;
}

double inline GetSpecificSurface(const std::vector<double>& _grid, const std::vector<double>& _q3)
{
	double dSV = 0;
	for (size_t i = 0; i < _q3.size(); i++)
		dSV += 6 * _q3[i] * 1 / ((_grid[i] + _grid[i + 1]) / 2) * (_grid[i + 1] - _grid[i]);
	return dSV;
}

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
