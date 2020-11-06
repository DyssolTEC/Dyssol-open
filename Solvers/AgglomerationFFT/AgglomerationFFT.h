/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "AgglomerationSolver.h"
#include "DyssolDefines.h"
#include "ThreadPool.h"
#include <complex>
#include "kiss_fftr.h"

#define DEFINE_AND_CHECK_STATIC_VECTOR(name, type, dim1, dim2) \
static std::vector<std::vector<type>> name; \
if (name.empty() || name.size() != dim1 || name.front().size() != dim2) { \
	name.clear(); \
	name.resize(dim1, std::vector<type>(dim2)); \
}

class CAgglomerationFFT : public CAgglomerationSolver
{
	size_t n;				// Number of intervals.
	size_t m_rank{ 3 };		// Rank is arbitrary.
	double m_dResizeFactor, m_dTransformFactor; // Scaling factors.

	std::vector<std::vector<double>> alpha, beta;
	std::vector<double> temp1, temp2;	// For precalculations.

	std::vector<kiss_fftr_cfg> m_FFTConfigF; // FFT solver configuration for each rank in forward direction.
	std::vector<kiss_fftr_cfg> m_FFTConfigB; // FFT solver configuration for each rank in backward direction.

public:
	void CreateBasicInfo() override;
	void Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams) override;
	bool Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate) override;
	void Finalize() override;

private:
	double BrownianAlpha(size_t _nu, double _v) const;
	double BrownianBeta(size_t _nu, double _v) const;

	void ApplyFFT(const std::vector<double>& _f, std::vector<double>& _BRate, std::vector<double>& _DRate);

	// Performs Fast Fourier Transformation (_bDirect = true) or Inverse Fast Fourier Transformation (_bDirect = false) on _data.
	void FFT(size_t _rank, std::vector<double>& _rData, std::vector<std::complex<double>>& _cData, bool _bDirect) const;

	static int sign(double E);
};
