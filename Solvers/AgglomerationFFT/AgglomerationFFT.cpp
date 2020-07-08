/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "AgglomerationFFT.h"
#include "DyssolUtilities.h"

extern "C" DECLDIR CAgglomerationSolver* CREATE_SOLVER_FUN_AGG1()
{
	return new CAgglomerationFFT();
}

CAgglomerationFFT::CAgglomerationFFT()
{
	m_solverName = "Agglomeration Solver FFT";
	m_authorName = "Lusine Shahmuradyan / Robin Ahrens";
	m_solverUniqueKey = "5547D68E93E844F8A55A36CB957A253B";
	m_solverVersion = 4;

	m_rank = 3;
}

void CAgglomerationFFT::Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams)
{
	m_beta0 = _beta0;
	m_kernel = _kernel;

	switch (m_kernel)
	{
	case EKernels::CONSTANT:	m_rank = 1; break;
	case EKernels::SUM: 		m_rank = 2;	break;
	case EKernels::BROWNIAN:	m_rank = 3;	break;
	default: 					m_rank = _rank;
	}

	if (_vGrid.empty()) return;
	n = _vGrid.size() - 1;
	const double Vmax = MATH_PI / 6. * std::pow(_vGrid.back(), 3.); // max volume
	const double h = 1.0 / n;

	for (size_t i = 0; i < m_rank; ++i)
	{
		m_FFTConfigF.push_back(kiss_fftr_alloc(int(n), 0, nullptr, nullptr));
		m_FFTConfigB.push_back(kiss_fftr_alloc(int(n), 1, nullptr, nullptr));
	}

	alpha.clear();
	beta.clear();
	alpha.resize(m_rank, std::vector<double>(n));
	beta.resize(m_rank, std::vector<double>(n));

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
			beta[0][j]  = 1;
			alpha[1][j] = 1;
			beta[1][j] = Vmax * h * (j + 0.5);
		}
		break;
	case EKernels::BROWNIAN:
		for (size_t j = 0; j < n; ++j)
		{
			alpha[0][j] = std::pow(Vmax * h * (j + 0.5), 1. / 3.);
			beta[0][j]  = std::pow(Vmax * h * (j + 0.5), -1. / 3.);
			alpha[1][j] = std::pow(Vmax * h * (j + 0.5), -1. / 3.);
			beta[1][j]  = std::pow(Vmax * h * (j + 0.5), 1. / 3.);
			alpha[2][j] = std::sqrt(2.);
			beta[2][j]  = std::sqrt(2.);
		}
		break;
	default: // Chebyshev interpolation
	{
		// Setup Space for Function Evaluations
		// E contains values with one parameter of grid-points
		std::vector<std::vector<std::vector<double>>> E(m_rank, std::vector<std::vector<double>>(m_rank, std::vector<double>(n)));
		// EZ contains values with both parameters at Chebyshev-Points
		std::vector<std::vector<std::vector<double>>> EZ(m_rank, std::vector<std::vector<double>>(m_rank));

		std::vector<double> ChebPoints(m_rank);
		ParallelFor(m_rank, [&](size_t r)
		{
			ChebPoints[r] = 0.5 + 0.5 * std::cos((2 * r + 1) * MATH_PI / (2 * m_rank));
		});


		// Evaluate Kernel function for all needed points
		ParallelFor(m_rank, [&](size_t r)
		{
			for (size_t i = 0; i < n; ++i)
				E[0][r][i] = Kernel(Vmax*h*(i + 0.5), Vmax*ChebPoints[r]);
			EZ[0][r].resize(m_rank);
			for (size_t r2 = 0; r2 < m_rank; ++r2)
				EZ[0][r][r2] = Kernel(Vmax*ChebPoints[r], Vmax*ChebPoints[r2]);
		});

		for (size_t i = 0; i < n; ++i)
		{
			alpha[0][i] = E[0][0][i];
			beta[0][i] = E[0][0][i] / EZ[0][0][0];
		}

		// Calculate error-functions of higher order
		// They are combinations of lower order evaluations
		for (size_t r = 1; r < m_rank; ++r)
		{
			E[r].resize(m_rank - r, std::vector<double>(n));
			for (size_t r2 = r; r2 < m_rank; ++r2)
				for (size_t i = 0; i < n; ++i)
					E[r][r2 - r][i] = E[r - 1][r2 - r + 1][i] - EZ[r - 1][0][r2 - r + 1] * E[r - 1][0][i] / EZ[r - 1][0][0];

			EZ[r].resize(m_rank - r);
			for (size_t r2 = 0; r2 < m_rank - r; ++r2)
			{
				EZ[r][r2].resize(m_rank - r);
				for (size_t r3 = 0; r3 < m_rank - r; ++r3)
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
		E[m_rank - 1].clear();
		EZ[m_rank - 1].clear();
	}
	}

	temp1.resize(n);
	temp2.resize(n);
	for (size_t i = 0; i < n; ++i)
	{
		temp1[i] = (i + 0.5 + 1. / 6.) / (2. * i + 1.);
		temp2[i] = (i + 0.5 - 1. / 6.) / (2. * i + 1.);
	}

	m_dResizeFactor = Vmax * Vmax * m_beta0;
	m_dTransformFactor = 1. / Vmax;
}

bool CAgglomerationFFT::Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate)
{
	_vBRate.assign(_vN.size(), 0);
	_vDRate.assign(_vN.size(), 0);
	if (_vN.empty()) return false;

	// initial distribution
	std::vector<double> f(n);
	for (size_t i = 0; i < n; ++i)
		f[i] = _vN[i] * m_dTransformFactor;

	ApplyFFT(f, _vBRate, _vDRate);

	for (size_t i = 0; i < n; ++i)
	{
		_vBRate[i] *= m_dResizeFactor;
		_vDRate[i] *= m_dResizeFactor;
	}

	return true;
}

void CAgglomerationFFT::Finalize()
{
	if (!m_FFTConfigF.empty())
		for (size_t i = 0; i < m_rank; ++i)
		{
			free(m_FFTConfigF[i]);
			free(m_FFTConfigB[i]);
		}
	m_FFTConfigF.clear();
	m_FFTConfigB.clear();
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

void CAgglomerationFFT::ApplyFFT(const std::vector<double>& _f, std::vector<double>& _BRate, std::vector<double>& _DRate)
{
	// Distribution and Kernel-Alpha
	DEFINE_AND_CHECK_STATIC_VECTOR(phi, double, m_rank, n);
	DEFINE_AND_CHECK_STATIC_VECTOR(PHI, std::complex<double>, m_rank, n / 2 + 1);

	// Distribution and Kernel-Beta
	DEFINE_AND_CHECK_STATIC_VECTOR(psi, double, m_rank, n);
	DEFINE_AND_CHECK_STATIC_VECTOR(PSI, std::complex<double>, m_rank, n / 2 + 1);

	// Pointwise Product of Fourier-Transform
	DEFINE_AND_CHECK_STATIC_VECTOR(omega, double, m_rank, n);
	DEFINE_AND_CHECK_STATIC_VECTOR(OMEGA, std::complex<double>, m_rank, n / 2 + 1);

	// Sink Term and Projection
	//DEFINE_AND_CHECK_STATIC_VECTOR(sink, double, m_rank, n + 1);
	DEFINE_AND_CHECK_STATIC_VECTOR(sink_proj, double, m_rank, n);

	// Source Term and projection
	DEFINE_AND_CHECK_STATIC_VECTOR(source, double, m_rank, n + 1);
	DEFINE_AND_CHECK_STATIC_VECTOR(source_proj, double, m_rank, n);

	ParallelFor(m_rank, [&](size_t nu)
	{
		for (size_t i = 0; i < n; ++i)
			phi[nu][i] = alpha[nu][i] * _f[i];

		for (size_t i = 0; i < n; ++i)
			psi[nu][i] = beta[nu][i] * _f[i];

		/* Here follows two variants of the sink-integral.
		The first is based on the Integral with upper limit 1-x
		The second is based on the Integral with upper limit 1
		If the Distribution is zero on [0.5, 1], there is no difference	*/

		/*Sink with ...*/
		//sink[nu][n] = 0.0;
		//for (int j = n - 1; j >= 0; --j)
		//	sink[nu][j] = sink[nu][j + 1] + psi[nu][n - 1 - j];

		//for (size_t i = 0; i < n; ++i)
		//	sink_proj[nu][i] = phi[nu][i] * (temp1[i] * sink[nu][i + 1] + temp2[i] * sink[nu][i]);
		/*limited Integral*/

		/*Sink with ... */
		double integral = 0;
		for (size_t i = 0; i < n; ++i)
			integral += psi[nu][i];

		for (size_t i = 0; i < n; ++i)
			sink_proj[nu][i] = phi[nu][i] * integral;
		/*...unlimited Integral*/

		/*Source Integral*/
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
	for (size_t nu = 0; nu < m_rank; ++nu)
		for (size_t i = 0; i < n; ++i)
		{
			_BRate[i] += 0.5 * source_proj[nu][i];
			_DRate[i] += sink_proj[nu][i];
		}
}

void CAgglomerationFFT::FFT(size_t _rank, std::vector<double>& _rData, std::vector<std::complex<double>>& _cData, bool _bDirect) const
{
	if (_bDirect)
		kiss_fftr(m_FFTConfigF[_rank], _rData.data(), reinterpret_cast<kiss_fft_cpx*>(_cData.data()));
	else
		kiss_fftri(m_FFTConfigB[_rank], reinterpret_cast<kiss_fft_cpx*>(_cData.data()), _rData.data());
}

int CAgglomerationFFT::sign(double E)
{
	return (0 < E) - (E < 0);
}
