/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "AgglomerationFixedPivot.h"

extern "C" DECLDIR CAgglomerationSolver* CREATE_SOLVER_FUN_AGG1()
{
	return new CAgglomerationFixedPivot();
}

void CAgglomerationFixedPivot::CreateBasicInfo()
{
	m_solverName = "Agglomeration Solver Fixed Pivot";
	m_authorName = "Lusine Shahmuradyan / Robin Ahrens";
	m_solverUniqueKey = "79E30F5FF8584FD2B773755F538FD8B2";
	m_solverVersion = 3;
}

void CAgglomerationFixedPivot::Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams)
{
	m_beta0 = _beta0;
	m_kernel = _kernel;

	if (_vGrid.empty()) return;
	n = _vGrid.size() - 1;
	m_pivotPoints.resize(_vGrid.size());
	for (size_t i = 0; i < _vGrid.size(); ++i)
		m_pivotPoints[i] = MATH_PI / 6. * std::pow(_vGrid[i], 3);

	// These two extra points are setup directly greater then the largest particle and the maximum size of an aggregation
	// This allows to catch all aggregations and ignore all too large ones
	m_pivotPoints.push_back(m_pivotPoints.back() + m_pivotPoints[1] * 0.5);
	m_pivotPoints.push_back(m_pivotPoints.back() * 2);

	m_kern.resize(n, std::vector<double>(n));
	m_target.resize(n, std::vector<size_t>(n));
	ParallelFor(n, [&](size_t i)
	{
		for (size_t j = 0; j < n; ++j)
		{
			m_kern[i][j] = Kernel(m_pivotPoints[i + 1], m_pivotPoints[j + 1]);
			m_target[i][j] = std::lower_bound(m_pivotPoints.begin(), m_pivotPoints.end(), m_pivotPoints[i + 1] + m_pivotPoints[j + 1]) - m_pivotPoints.begin() - 1;
		}
	});
}

bool CAgglomerationFixedPivot::Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate)
{
	_vBRate.assign(_vN.size(), 0);
	_vDRate.assign(_vN.size(), 0);
	if (_vN.empty()) return false;

	ApplyFixedPivot(_vN, _vBRate, _vDRate);

	for (size_t i = 0; i < n; ++i)
	{
		_vBRate[i] *= m_beta0;
		_vDRate[i] *= m_beta0;
	}

	return true;
}

void CAgglomerationFixedPivot::ApplyFixedPivot(const std::vector<double>& _f, std::vector<double>& _BRate, std::vector<double>& _DRate)
{
	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j <= i; ++j)
		{
			const double val = _f[i] * _f[j] * m_kern[i][j] * (1 - (i == j) * 0.5);

			//sink
			_DRate[i] += val;
			_DRate[j] += val;

			//source
			const double k = m_pivotPoints[i + 1] + m_pivotPoints[j + 1];
			const size_t t = m_target[i][j];
			const double q1 = (m_pivotPoints[t + 1] - k) / (m_pivotPoints[t + 1] - m_pivotPoints[t]);
			const double q2 = (k - m_pivotPoints[t]) / (m_pivotPoints[t + 1] - m_pivotPoints[t]);
			if (t - 1 < n)
				_BRate[t - 1] += q1 * val;
			if (t < n)
				_BRate[t] += q2 * val;
		}
}
