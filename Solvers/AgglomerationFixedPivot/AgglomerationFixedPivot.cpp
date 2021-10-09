/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "AgglomerationFixedPivot.h"
#include "DyssolDefines.h"

extern "C" DECLDIR CAgglomerationSolver* CREATE_SOLVER_FUN_AGG1()
{
	return new CAgglomerationFixedPivot();
}

void CAgglomerationFixedPivot::CreateBasicInfo()
{
	SetName("Fixed Pivot");
	SetAuthorName("Lusine Shahmuradyan / Robin Ahrens");
	SetUniqueID("79E30F5FF8584FD2B773755F538FD8B2");
	SetVersion(3);
}

void CAgglomerationFixedPivot::Initialize()
{
	n = m_grid.size() - 1;
	pivotPoints.resize(m_grid.size());
	for (size_t i = 0; i < m_grid.size(); ++i)
		pivotPoints[i] = MATH_PI / 6. * std::pow(m_grid[i], 3);

	// These two extra points are setup directly greater then the largest particle and the maximum size of an aggregation
	// This allows to catch all aggregations and ignore all too large ones
	pivotPoints.push_back(pivotPoints.back() + pivotPoints[1] * 0.5);
	pivotPoints.push_back(pivotPoints.back() * 2);

	beta.resize(n, d_vect_t(n));
	target.resize(n, u_vect_t(n));
	ParallelFor(n, [&](size_t i)
	{
		for (size_t j = 0; j < n; ++j)
		{
			beta[i][j] = Kernel(pivotPoints[i + 1], pivotPoints[j + 1]);
			target[i][j] = std::lower_bound(pivotPoints.begin(), pivotPoints.end(), pivotPoints[i + 1] + pivotPoints[j + 1]) - pivotPoints.begin() - 1;
		}
	});
}

void CAgglomerationFixedPivot::Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD)
{
	_rateB.assign(_n.size(), 0.0);
	_rateD.assign(_n.size(), 0.0);
	if (_n.empty()) return;

	ApplyFixedPivot(_n, _rateB, _rateD);

	for (size_t i = 0; i < n; ++i)
	{
		_rateB[i] *= m_beta0;
		_rateD[i] *= m_beta0;
	}
}

void CAgglomerationFixedPivot::ApplyFixedPivot(const d_vect_t& _f, d_vect_t& _rateB, d_vect_t& _rateD)
{
	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j <= i; ++j)
		{
			const double val = _f[i] * _f[j] * beta[i][j] * (1 - (i == j) * 0.5);

			//sink
			_rateD[i] += val;
			_rateD[j] += val;

			//source
			const double k = pivotPoints[i + 1] + pivotPoints[j + 1];
			const size_t t = target[i][j];
			const double q1 = (pivotPoints[t + 1] - k) / (pivotPoints[t + 1] - pivotPoints[t]);
			const double q2 = (k - pivotPoints[t]) / (pivotPoints[t + 1] - pivotPoints[t]);
			if (t - 1 < n)
				_rateB[t - 1] += q1 * val;
			if (t < n)
				_rateB[t] += q2 * val;
		}
}
