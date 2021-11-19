/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "AgglomerationSolver.h"
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
	size_t n{};					// Number of size-intervals.
	size_t rank{ 3 };			// Separation rank.
	double resizeFactor{};		// Scaling factor.
	double transformFactor{};	// Scaling factor.

	d_matr_t alpha, beta;
	d_vect_t temp1, temp2;	// For precalculations.

	std::vector<kiss_fftr_cfg> fftConfigF; // FFT solver configuration for each rank in forward direction.
	std::vector<kiss_fftr_cfg> fftConfigB; // FFT solver configuration for each rank in backward direction.

public:
	void CreateBasicInfo() override;
	void Initialize() override;
	void Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD) override;
	void Finalize() override;

private:
	double BrownianAlpha(size_t _nu, double _v) const;
	double BrownianBeta(size_t _nu, double _v) const;

	void ApplyFFT(const d_vect_t& _f, d_vect_t& _rateB, d_vect_t& _rateD);

	// Performs Fast Fourier Transformation (_bDirect = true) or Inverse Fast Fourier Transformation (_bDirect = false) on _data.
	void FFT(size_t _rank, d_vect_t& _rData, std::vector<std::complex<double>>& _cData, bool _direct) const;
};
