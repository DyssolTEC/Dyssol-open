/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "BaseSolver.h"
#include <functional>

class CAgglomerationSolver : public CBaseSolver
{
public:
	enum class EKernels : size_t
	{
		CONSTANT      = 0,
		SUM           = 1,
		PRODUCT       = 2,
		BROWNIAN      = 3,
		SHEAR         = 4,
		PEGLOW        = 5,
		COAGULATION   = 6,
		GRAVITATIONAL = 7,
		EKE           = 8,
		THOMPSON      = 9,
		CUSTOM        = 10,
	};

protected:
	using u_matr_t = std::vector<std::vector<size_t>>;
	using d_matr_t = std::vector<std::vector<double>>;
	using u_vect_t = std::vector<size_t>;
	using d_vect_t = std::vector<double>;

	using kernel_t = double(double, double);	// Type of the kernel function.

	EKernels m_kernel{ EKernels::BROWNIAN };	// Selected kernel function.
	double m_beta0{ 1.0 };						// Size independent agglomeration rate (set zero for no agglomeration).
	std::vector<double> m_parameters;			// Additional parameters.
	std::vector<double> m_grid;					// Diameter-related PSD grid.
	std::function<kernel_t> m_CutomKernel{};	// Custom kernel function.

public:
	CAgglomerationSolver();
	~CAgglomerationSolver() override                                     = default;
	CAgglomerationSolver(const CAgglomerationSolver& _other)             = default;
	CAgglomerationSolver(CAgglomerationSolver && _other)                 = default;
	CAgglomerationSolver& operator=(const CAgglomerationSolver & _other) = default;
	CAgglomerationSolver& operator=(CAgglomerationSolver && _other)      = default;

	/// <summary>
	/// Sets all required parameters and calls Initialize()
	/// </summary>
	/// <param name="_grid">Diameter-related PSD grid</param>
	/// <param name="_beta0">Size independent agglomeration rate</param>
	/// <param name="_kernel">Type of the agglomeration kernel</param>
	/// <param name="_parameters">Additional parameters</param>
	void Initialize(const d_vect_t& _grid, double _beta0, EKernels _kernel, const d_vect_t& _parameters = {});
	/// <summary>
	/// Sets all required parameters and calls Initialize()
	/// </summary>
	/// <param name="_grid">Diameter-related PSD grid</param>
	/// <param name="_beta0">Size independent agglomeration rate</param>
	/// <param name="_kernel">Function of the agglomeration kernel</param>
	/// <param name="_parameters">Additional parameters</param>
	void Initialize(const d_vect_t& _grid, double _beta0, const std::function<kernel_t>& _kernel, const d_vect_t& _parameters = {});

	/// <summary>
	/// Actual initialization of the solver.
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// Main calculation function
	/// </summary>
	/// <param name="_n">Number distribution</param>
	/// <param name="_rateB">Output vector for birth rate</param>
	/// <param name="_rateD">Output vector for death rate</param>
	virtual void Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD);
	/// <summary>
	/// Main calculation function
	/// </summary>
	/// <param name="_n">Number distribution</param>
	/// <returns>Birth and death rates</returns>
	std::pair<d_vect_t, d_vect_t> Calculate(const d_vect_t& _n);

protected:
	// Calculates the chosen kernel function for particles with volumes _u and _v.
	[[nodiscard]] double Kernel(double _u, double _v) const;

private:
	// Sets all parameters.
	void SetParameters(const d_vect_t& _grid, double _beta0, EKernels _kernel, const std::function<kernel_t>& _kernelFun, const d_vect_t& _parameters);
};

typedef DECLDIR CAgglomerationSolver* (*CreateAgglomerationSolver)();
