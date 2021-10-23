/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "AgglomerationCellAverage.h"
#include "DyssolDefines.h"
#include <cmath>

extern "C" DECLDIR CAgglomerationSolver* CREATE_SOLVER_FUN_AGG1()
{
	return new CAgglomerationCellAverage();
}

void CAgglomerationCellAverage::CreateBasicInfo()
{
	SetName("Cell Average");
	SetAuthorName("Robin Ahrens");
	SetUniqueID("B1C8328850C34E27BE4D338AEE39B2B4");
	SetVersion(3);
}

void CAgglomerationCellAverage::Initialize()
{
	n = m_grid.size() - 1;
	const double h = 1.0 / static_cast<double>(n);					// size of interval
	const double Vmax = MATH_PI / 6. * std::pow(m_grid.back(), 3);	// max volume

	beta.resize(n, d_vect_t(n));
	ParallelFor(n, [&](size_t i)
	{
		for (size_t j = 0; j < n; ++j)
			beta[i][j] = Kernel(Vmax * (h * i + h / 2.0), Vmax * (h * j + h / 2.0));
	});
}

void CAgglomerationCellAverage::Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD)
{
	_rateB.assign(_n.size(), 0.0);
	_rateD.assign(_n.size(), 0.0);
	if (_n.empty()) return;

	ApplyCellAverage(_n, _rateB, _rateD);

	for (size_t j = 0; j < n; ++j)
	{
		_rateB[j] *= m_beta0;
		_rateD[j] *= m_beta0;
	}
}

void CAgglomerationCellAverage::ApplyCellAverage(const d_vect_t& _f, d_vect_t& _rateB, d_vect_t& _rateD)
{
	d_vect_t avg(n, 0.0);
	d_vect_t b(n, 0.0);

	ParallelFor(n, [&](size_t i)
	{
		for (size_t j = 0; j < n - i - 1; ++j)
			_rateD[i] += beta[i][j] * _f[i] * _f[j];

		for (size_t j = 0; j < i; ++j)
		{
			const double val = 0.5 * _f[i - j - 1] * _f[j] * beta[i - j - 1][j];
			b[i] += val;
			avg[i] += val * i;
		}

		avg[i] = b[i] != 0.0 ? avg[i] / b[i] : 0.0;

	});

	_rateB[0] += b[0] * (1 - avg[0]) * Heavyside(avg[0]);
	_rateB[0] += b[1] * (1 - avg[1]) * Heavyside(1 - avg[1]);

	for (size_t i = 1; i < n - 1; ++i)
	{
		_rateB[i] += b[i - 1] * (avg[i - 1] - i + 1) * Heavyside(avg[i - 1] - i + 1);
		_rateB[i] += b[i] * (avg[i] - i + 1) * Heavyside(i - avg[i]);
		_rateB[i] += b[i] * (i + 1 - avg[i]) * Heavyside(avg[i] - i);
		_rateB[i] += b[i + 1] * (i + 1 - avg[i + 1]) * Heavyside(i + 1 - avg[i + 1]);
	}

	_rateB[n - 1] += b[n - 2] * (avg[n - 2] - n) * Heavyside(avg[n - 2] - n);
	_rateB[n - 1] += b[n - 1] * (avg[n - 1] - n) * Heavyside(n - 1 - avg[n - 1]);
}

double CAgglomerationCellAverage::Heavyside(double _v)
{
	return _v > 0.0 ? 1.0 : _v == 0.0 ? 0.5 : 0.0;
}
