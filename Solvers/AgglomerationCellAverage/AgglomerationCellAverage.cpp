/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "AgglomerationCellAverage.h"

extern "C" DECLDIR CAgglomerationSolver* CREATE_SOLVER_FUN_AGG1()
{
	return new CAgglomerationCellAverage();
}

CAgglomerationCellAverage::CAgglomerationCellAverage()
{
	m_solverName = "Agglomeration Solver Cell Average";
	m_authorName = "Robin Ahrens";
	m_solverUniqueKey = "B1C8328850C34E27BE4D338AEE39B2B4";
	m_solverVersion = 3;
}

void CAgglomerationCellAverage::Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams)
{
	m_beta0 = _beta0;
	m_kernel = _kernel;

	if (_vGrid.empty()) return;
	n = _vGrid.size() - 1;
	const double h = 1.0 / n;										// size of interval
	const double Vmax = MATH_PI / 6. * std::pow(_vGrid.back(), 3);	// max volume

	kern.resize(n, std::vector<double>(n));
	ParallelFor(n, [&](size_t i)
	{
		for (size_t j = 0; j < n; ++j)
			kern[i][j] = Kernel(Vmax * (h * i + h / 2.0), Vmax * (h * j + h / 2.0));
	});
}

bool CAgglomerationCellAverage::Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate)
{
	_vBRate.assign(_vN.size(), 0);
	_vDRate.assign(_vN.size(), 0);
	if (_vN.empty()) return false;

	ApplyCellAverage(_vN, _vBRate, _vDRate);

	for (size_t j = 0; j < n; ++j)
	{
		_vBRate[j] *= m_beta0;
		_vDRate[j] *= m_beta0;
	}

	return true;
}

void CAgglomerationCellAverage::ApplyCellAverage(const std::vector<double>& _f, std::vector<double>& _BRate, std::vector<double>& _DRate)
{
	std::vector<double> avg(n, 0.);
	std::vector<double> b(n, 0.);

	ParallelFor(n, [&](size_t i)
	{
		for (size_t j = 0; j < n - i - 1; ++j)
			_DRate[i] += kern[i][j] * _f[i] * _f[j];

		for (size_t j = 0; j < i; ++j)
		{
			const double val = 0.5 * _f[i - j - 1] * _f[j] * kern[i - j - 1][j];
			b[i] += val;
			avg[i] += val * i;
		}

		avg[i] = b[i] != 0 ? avg[i] / b[i] : 0;

	});

	_BRate[0] += b[0] * (1 - avg[0]) * Heavyside(avg[0]);
	_BRate[0] += b[1] * (1 - avg[1]) * Heavyside(1 - avg[1]);

	for (size_t i = 1; i < n - 1; ++i)
	{
		_BRate[i] += b[i - 1] * (avg[i - 1] - i + 1) * Heavyside(avg[i - 1] - i + 1);
		_BRate[i] += b[i] * (avg[i] - i + 1) * Heavyside(i - avg[i]);
		_BRate[i] += b[i] * (i + 1 - avg[i]) * Heavyside(avg[i] - i);
		_BRate[i] += b[i + 1] * (i + 1 - avg[i + 1]) * Heavyside(i + 1 - avg[i + 1]);
	}

	_BRate[n - 1] += b[n - 2] * (avg[n - 2] - n) * Heavyside(avg[n - 2] - n);
	_BRate[n - 1] += b[n - 1] * (avg[n - 1] - n) * Heavyside(n - 1 - avg[n - 1]);
}

double CAgglomerationCellAverage::Heavyside(double v)
{
	return v > 0 ? 1 : v == 0 ? 0.5 : 0;
}
