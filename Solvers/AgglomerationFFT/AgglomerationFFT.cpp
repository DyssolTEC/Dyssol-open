/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "AgglomerationFFT.h"
#include "DyssolDefines.h"
#include "ThreadPool.h"

extern "C" DECLDIR CAgglomerationSolver * CREATE_SOLVER_FUN_AGG1()
{
	return new CAgglomerationFFT();
}

void CAgglomerationFFT::CreateBasicInfo()
{
	SetName("FFT");
	SetAuthorName("Lusine Shahmuradyan / Robin Ahrens");
	SetUniqueID("5547D68E93E844F8A55A36CB957A253B");
	SetVersion(4);
	SetHelpLink("003_models/solver_fft.html");
}

void CAgglomerationFFT::Initialize()
{
	switch (m_kernel)
	{
	case EKernels::CONSTANT:	rank = 1; break;
	case EKernels::SUM: 		rank = 2;	break;
	case EKernels::BROWNIAN:	rank = 3;	break;
	default:
	{
		if (!m_parameters.empty())
			rank = static_cast<size_t>(m_parameters[0]);
		// TODO:
		//else
			//ShowInfo("Using default rank: " + std::to_string(rank) + ".");
	}
	}

	n = m_grid.size() - 1;
	const double Vmax = MATH_PI / 6. * std::pow(m_grid.back(), 3.); // max volume
	const double h = 1.0 / n;

	for (size_t i = 0; i < rank; ++i)
	{
		fftConfigF.push_back(kiss_fftr_alloc(static_cast<int>(n), 0, nullptr, nullptr));
		fftConfigB.push_back(kiss_fftr_alloc(static_cast<int>(n), 1, nullptr, nullptr));
	}

	alpha.clear();
	beta.clear();
	alpha.resize(rank, d_vect_t(n));
	beta.resize(rank, d_vect_t(n));

	switch (m_kernel)
	{
	case EKernels::CONSTANT:
		for (size_t j = 0; j < n; ++j)
		{
			alpha[0][j] = Vmax * h * (j + 0.5);
			beta[0][j] = 1;
		}
		break;
	case EKernels::SUM:
		for (size_t j = 0; j < n; ++j)
		{
			alpha[0][j] = Vmax * h * (j + 0.5);
			beta[0][j] = 1;
			alpha[1][j] = 1;
			beta[1][j] = Vmax * h * (j + 0.5);
		}
		break;
	case EKernels::BROWNIAN:
		for (size_t j = 0; j < n; ++j)
		{
			alpha[0][j] = std::pow(Vmax * h * (j + 0.5), 1. / 3.);
			beta[0][j] = std::pow(Vmax * h * (j + 0.5), -1. / 3.);
			alpha[1][j] = std::pow(Vmax * h * (j + 0.5), -1. / 3.);
			beta[1][j] = std::pow(Vmax * h * (j + 0.5), 1. / 3.);
			alpha[2][j] = std::sqrt(2.);
			beta[2][j] = std::sqrt(2.);
		}
		break;
	default: // Chebyshev interpolation
	{
		// Setup Space for Function Evaluations
		// E contains values with one parameter of grid-points
		std::vector<d_matr_t> E(rank, d_matr_t(rank, d_vect_t(n)));
		// EZ contains values with both parameters at Chebyshev-Points
		std::vector<d_matr_t> EZ(rank, d_matr_t(rank));

		d_vect_t ChebPoints(rank);
		ParallelFor(rank, [&](size_t r)
		{
			ChebPoints[r] = 0.5 + 0.5 * std::cos((2 * r + 1) * MATH_PI / (2 * rank));
		});


		// Evaluate Kernel function for all needed points
		ParallelFor(rank, [&](size_t r)
		{
			for (size_t i = 0; i < n; ++i)
				E[0][r][i] = Kernel(Vmax * h * (i + 0.5), Vmax * ChebPoints[r]);
			EZ[0][r].resize(rank);
			for (size_t r2 = 0; r2 < rank; ++r2)
				EZ[0][r][r2] = Kernel(Vmax * ChebPoints[r], Vmax * ChebPoints[r2]);
		});

		for (size_t i = 0; i < n; ++i)
		{
			alpha[0][i] = E[0][0][i];
			beta[0][i] = E[0][0][i] / EZ[0][0][0];
		}

		// Calculate error-functions of higher order
		// They are combinations of lower order evaluations
		for (size_t r = 1; r < rank; ++r)
		{
			E[r].resize(rank - r, d_vect_t(n));
			for (size_t r2 = r; r2 < rank; ++r2)
				for (size_t i = 0; i < n; ++i)
					E[r][r2 - r][i] = E[r - 1][r2 - r + 1][i] - EZ[r - 1][0][r2 - r + 1] * E[r - 1][0][i] / EZ[r - 1][0][0];

			EZ[r].resize(rank - r);
			for (size_t r2 = 0; r2 < rank - r; ++r2)
			{
				EZ[r][r2].resize(rank - r);
				for (size_t r3 = 0; r3 < rank - r; ++r3)
					EZ[r][r2][r3] = EZ[r - 1][r2 + 1][r3 + 1] - EZ[r - 1][0][r3 + 1] * EZ[r - 1][r2 + 1][0] / EZ[r - 1][0][0];
			}
			E[r - 1].clear();
			EZ[r - 1].clear();
			for (size_t i = 0; i < n; ++i)
			{
				alpha[r][i] = E[r][0][i];
				beta[r][i] = E[r][0][i] / EZ[r][0][0];
			}
		}
		E[rank - 1].clear();
		EZ[rank - 1].clear();
	}
	}

	temp1.resize(n);
	temp2.resize(n);
	for (size_t i = 0; i < n; ++i)
	{
		temp1[i] = (i + 0.5 + 1. / 6.) / (2. * i + 1.);
		temp2[i] = (i + 0.5 - 1. / 6.) / (2. * i + 1.);
	}

	resizeFactor = Vmax * Vmax * m_beta0;
	transformFactor = 1. / Vmax;
}

void CAgglomerationFFT::Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD)
{
	_rateB.assign(_n.size(), 0);
	_rateD.assign(_n.size(), 0);
	if (_n.empty()) return;

	// initial distribution
	d_vect_t f(n);
	for (size_t i = 0; i < n; ++i)
		f[i] = _n[i] * transformFactor;

	ApplyFFT(f, _rateB, _rateD);

	for (size_t i = 0; i < n; ++i)
	{
		_rateB[i] *= resizeFactor;
		_rateD[i] *= resizeFactor;
	}
}

void CAgglomerationFFT::Finalize()
{
	if (!fftConfigF.empty())
		for (size_t i = 0; i < rank; ++i)
		{
			free(fftConfigF[i]);
			free(fftConfigB[i]);
		}
	fftConfigF.clear();
	fftConfigB.clear();
}

double CAgglomerationFFT::BrownianAlpha(size_t _nu, double _v) const
{
	if (_nu == 0)	return std::pow(_v, 1. / 3.);
	if (_nu == 1)	return std::pow(_v, -1. / 3.);
	if (_nu == 2)	return std::sqrt(2.);
	return 0;
}

double CAgglomerationFFT::BrownianBeta(size_t _nu, double _v) const
{
	if (_nu == 0)	return std::pow(_v, -1. / 3.);
	if (_nu == 1)	return std::pow(_v, 1. / 3.);
	if (_nu == 2)	return std::sqrt(2.);
	return 0;
}

void CAgglomerationFFT::ApplyFFT(const d_vect_t& _f, d_vect_t& _rateB, d_vect_t& _rateD)
{
	// Distribution and Kernel-Alpha
	DEFINE_AND_CHECK_STATIC_VECTOR(phi, double, rank, n);
	DEFINE_AND_CHECK_STATIC_VECTOR(PHI, std::complex<double>, rank, n / 2 + 1);

	// Distribution and Kernel-Beta
	DEFINE_AND_CHECK_STATIC_VECTOR(psi, double, rank, n);
	DEFINE_AND_CHECK_STATIC_VECTOR(PSI, std::complex<double>, rank, n / 2 + 1);

	// Pointwise Product of Fourier-Transform
	DEFINE_AND_CHECK_STATIC_VECTOR(omega, double, rank, n);
	DEFINE_AND_CHECK_STATIC_VECTOR(OMEGA, std::complex<double>, rank, n / 2 + 1);

	// Sink Term and Projection
	//DEFINE_AND_CHECK_STATIC_VECTOR(sink, double, m_rank, n + 1);
	DEFINE_AND_CHECK_STATIC_VECTOR(sink_proj, double, rank, n);

	// Source Term and projection
	DEFINE_AND_CHECK_STATIC_VECTOR(source, double, rank, n + 1);
	DEFINE_AND_CHECK_STATIC_VECTOR(source_proj, double, rank, n);

	ParallelFor(rank, [&](size_t nu)
	{
		for (size_t i = 0; i < n; ++i)
			phi[nu][i] = alpha[nu][i] * _f[i];

		for (size_t i = 0; i < n; ++i)
			psi[nu][i] = beta[nu][i] * _f[i];

		/* Here follows two variants of the sink-integral.
		The first is based on the Integral with upper limit 1-x
		The second is based on the Integral with upper limit 1
		If the Distribution is zero on [0.5, 1], there is no difference	*/

		/* Sink with ... */
		//sink[nu][n] = 0.0;
		//for (int j = n - 1; j >= 0; --j)
		//	sink[nu][j] = sink[nu][j + 1] + psi[nu][n - 1 - j];

		//for (size_t i = 0; i < n; ++i)
		//	sink_proj[nu][i] = phi[nu][i] * (temp1[i] * sink[nu][i + 1] + temp2[i] * sink[nu][i]);
		/* limited Integral */

		/* Sink with ... */
		double integral = 0;
		for (size_t i = 0; i < n; ++i)
			integral += psi[nu][i];

		for (size_t i = 0; i < n; ++i)
			sink_proj[nu][i] = phi[nu][i] * integral;
		/* ...unlimited Integral */

		/* Source Integral */
		std::fill(PHI[nu].begin(), PHI[nu].end(), std::complex<double>(0.0, 0.0));
		std::fill(PSI[nu].begin(), PSI[nu].end(), std::complex<double>(0.0, 0.0));

		FFT(nu, phi[nu], PHI[nu], true);
		FFT(nu, psi[nu], PSI[nu], true);

		for (size_t i = 0; i < OMEGA[nu].size(); ++i)
			OMEGA[nu][i] = PHI[nu][i] * PSI[nu][i];

		FFT(nu, omega[nu], OMEGA[nu], false);

		source[nu][0] = 0.0;
		for (size_t i = 1; i < n + 1; ++i)
			source[nu][i] = omega[nu][i - 1] / n;

		for (size_t i = 0; i < n; ++i)
			source_proj[nu][i] = temp1[i] * source[nu][i + 1] + temp2[i] * source[nu][i];
	});

	// output
	for (size_t nu = 0; nu < rank; ++nu)
		for (size_t i = 0; i < n; ++i)
		{
			_rateB[i] += 0.5 * source_proj[nu][i];
			_rateD[i] += sink_proj[nu][i];
		}
}

void CAgglomerationFFT::FFT(size_t _rank, d_vect_t& _rData, std::vector<std::complex<double>>& _cData, bool _direct) const
{
	if (_direct)
		kiss_fftr(fftConfigF[_rank], _rData.data(), reinterpret_cast<kiss_fft_cpx*>(_cData.data()));
	else
		kiss_fftri(fftConfigB[_rank], reinterpret_cast<kiss_fft_cpx*>(_cData.data()), _rData.data());
}
